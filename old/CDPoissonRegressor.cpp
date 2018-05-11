#include <iostream>

#include "CDPoissonRegressor.h"

CDPoissonRegressor::CDPoissonRegressor(Config* config, Dataset* dataset):
    ALinearRegressor(config, dataset)
{
    columns = new uint8_t*[nbCoeffs + 1];
    for(int i = 0; i < nbCoeffs + 1; i++){
        columns[i] = NULL;
    }
    columns[0] = new uint8_t[dataset->train.size()];
    uint8_t* intercept = columns[0];
    for(int i = 0; i < dataset->train.size(); i++){
        intercept[i] = 1;
    }
}

std::vector<int> argsort(const std::vector<double> &v) {
  // initialize original index locations
  std::vector<int> idx(v.size());
  iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return std::abs(v[i1]) > std::abs(v[i2]);});

  return idx;
}

uint8_t* CDPoissonRegressor::cacheColumn(int m){
    if(columns[m] == NULL){
        //std::cout << "caching " << m << std::endl;
        int f = config->getFeatureFromModality(m - 1);
        columns[m] = new uint8_t[dataset->train.size()];
        uint8_t* column = columns[m];
        for(int k = 0; k < dataset->train.size(); k++){
            int i = dataset->train[k];
            uint8_t* xi = x + p * i;
            column[k] = (offsets[f] + xi[f] + 1 == m);
        }
        //std::cout << "end caching " << m << std::endl;
    }
    return columns[m];
}

void CDPoissonRegressor::fit(int blocksize, float learning_rate){
    std::cout << "precomputation" << std::endl;
    std::vector<int> sample(blocksize);

    /*for(int k = 0; k < blocksize; k++){
        sample[k] = dataset->next();
    }*/
    sample = dataset->train;
    blocksize = sample.size();

    std::vector<double> ypred(blocksize);
    std::vector<double> xy(nbCoeffs + 1);
    std::vector<double> grad(nbCoeffs + 1);

    double dp0 = coeffs[0];
    for(int m = 0; m < nbCoeffs + 1 ; m++){
        xy[m] = 0;
        dp0 += x0[m] * coeffs[m];
        grad[m] = 0;
    }
    double rTotal = 0;
    for(int k = 0; k < blocksize; k++){
        int i = sample[k];
        uint8_t* xi = x + p * i;
        xy[0] += y[i];
        for(int m = 0; m < nbCoeffs + 1; m++){
            xy[m] += x0[m] * y[i];
        }
        double dp = dp0;
        for(int j : selected_features){
            int l = offsets[j]+ xi[j] + 1;
            dp += (x1[l] - x0[l]) * coeffs[l];
            xy[l] += (x1[l] - x0[l]) * y[i];
        }
        ypred[k] = std::exp(dp) * exposure[i];
        double r = y[k]- ypred[k];
        rTotal += r;
        for(int j = 0; j < p ; j++){
            int k = offsets[j]+ xi[j] + 1;
            grad[k] += r * (x1[k] - x0[k]);
        }
    }
    for(int m = 0; m < nbCoeffs + 1 ; m++){
        grad[m] += rTotal * x0[m];
    }

    std::vector<int> sortedGrad = argsort(grad);
    sortedGrad.insert(sortedGrad.begin(), 0);
    /*
    for(auto m : sortedGrad){
        std::cout << m << " " << config->features[config->getFeatureFromModality(m-1)] << " " << grad[m] << std::endl;
    }
    */

    std::cout << "fit" << std::endl;
    //for(int m = 0; m < nbCoeffs + 1; m++){
    //for(int m : sortedGrad){
    for(int g = 0; g < nbCoeffs / 2; g++){
        int m = sortedGrad[g];
        int f = m > 0 ? config->getFeatureFromModality(m-1) : 0;
        if(m > 0 && selected_features.find(f) == selected_features.end()) continue;
        //std::cout << "m=" << m << ", f=" << f << std::endl;
        if(weights[m] < std::sqrt(weights[0]) / 10){
            // squeezing non significative coefficients to Zero
            coeffs[m] = 0; // this line is not required (just to be explicit) !
            continue;
        }
        uint8_t* column = cacheColumn(m);

        double A = 0;
        double B = 0;
        double C = xy[m];

        double xm0 = x0[m];
        double xm1 = x1[m];

        for(int k = 0; k < blocksize; k++){
            int xim = column[k];
            double yp = ypred[k];
            B += xim * xm1 * yp;
            A += (1 - xim) * xm0 * yp;
        }

        if(m != 0 && (A == 0 || B == 0)){
            continue;
        }

        double a = 0;
        double r = A + B - C;
        while(std::abs(r) > 0.001){
            a -= r / (A * xm0 * std::exp(a * xm0) + B * xm1 * std::exp(a * xm1));
            r = A * std::exp(a * xm0) + B * std::exp(a * xm1) - C;
            //std::cout << "a=" << a << ", r=" << r << ", A=" << A << ", B=" << B << ", C=" << C << " x0=" << x0[m] << " x1=" << x1[m] << std::endl;
        }
        coeffs[m] += a;
        //std::cout << config->features[f] << ":" << m <<  " coeff : " << coeffs[m] << " s= " << stdev[m] << " x0=" << x0[m] << " x1=" << x1[m] << std::endl;
        for(int k = 0; k < blocksize; k++){
            int xim = column[k];
            double xf = xim * xm1 + (1 - xim) * xm0;
            ypred[k] *= std::exp(xf * a);
        }
    }
    std::cout << "end" << std::endl;
}

