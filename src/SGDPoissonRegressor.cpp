#include <iostream>

#include "SGDPoissonRegressor.h"

SGDPoissonRegressor::SGDPoissonRegressor(int p, int n,
    uint8_t* x, float* y, float* exposure, int nbCoeffs,
    const std::vector<int> &offsets, std::vector<std::string> &features) : ALinearRegressor(p, n, x, y, exposure, nbCoeffs, offsets, features)
{
    update.reserve(nbCoeffs + 1);
}

void SGDPoissonRegressor::fit(int i, float learning_rate)
{
    uint8_t* xi = x + p * i;

    double r = learning_rate * (y[i]- pred(i));
    coeffs[0] += r;
    for(int j = 0; j < p ; j++){
        coeffs[offsets[j]+ xi[j] + 1] += r;
    }
}

void SGDPoissonRegressor::filterfit(int i, float learning_rate, std::set<int> &feature_filters)
{
    uint8_t* xi = x + p * i;

    double r = learning_rate * (y[i]- pred(i));

    coeffs[0] += r;
    for(int j = 1; j < nbCoeffs + 1; j++){
        coeffs[j] += r * x0[j];
    }
    for(int j : feature_filters){
        int k = offsets[j]+ xi[j] + 1;
        coeffs[k] += r * (x1[k] - x0[k]);
    }
}

void SGDPoissonRegressor::blockfit(Dataset &ds, int blocksize, float learning_rate, std::set<int> &feature_filters){
    double dp0 = coeffs[0];
    for(int j = 1; j < nbCoeffs + 1 ; j++){
        dp0 += x0[j] * coeffs[j];
        update[j] = 0;
    }

    double rTotal = 0;
    for(int k = 0; k < blocksize; k++){
        int i = ds.next();
        uint8_t* xi = x + p * i;

        double dp = dp0;
        for(int j : feature_filters){
            int k = offsets[j]+ xi[j] + 1;
            dp += (x1[k] - x0[k]) * coeffs[k];
        }
        double ypred = exp(dp) * exposure[i];
        double r = y[i]- ypred;
        rTotal += r;
        for(int j = 0; j < p ; j++){
            int k = offsets[j]+ xi[j] + 1;
            update[k] += r * (x1[k] - x0[k]);
        }
    }

    coeffs[0] += rTotal * learning_rate / blocksize;
    for(int j = 1; j < nbCoeffs + 1 ; j++){
        double w = weights[j];
        if(w < std::sqrt(n) / 100){
            // squeezing non significative coefficients to Zero
            coeffs[j] = 0; // this line is not required (just to be explicit) !
            continue;
        }
        coeffs[j] += (update[j] + rTotal * x0[j]) * learning_rate / blocksize;
    }
}

void SGDPoissonRegressor::squeezeCoeffs(){
    for(int j = 1; j < nbCoeffs + 1 ; j++){
        double w = weights[j];
        if(w < std::sqrt(n)){
            coeffs[j] = 0;
        }
    }
}
