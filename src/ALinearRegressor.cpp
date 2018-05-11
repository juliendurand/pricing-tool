#include <algorithm>
#include <assert.h>
#include <cinttypes>
#include <cmath>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "array.h"
#include "ALinearRegressor.h"


ALinearRegressor::ALinearRegressor(Config* configuration, Dataset* ds)
{
    config = configuration;
    dataset = ds;
    n = config->n;
    p = config->p;
    nbCoeffs = config->m;
    offsets = config->offsets;
    features = config->features;
    x = dataset->x_data->getData();
    exposure = dataset->weight_data->getData();
    y = dataset->y_data->getData();

    coeffs = new double[nbCoeffs + 1];
    weights = new double[nbCoeffs + 1];
    stdev = new double[nbCoeffs + 1];
    x0 = new double[nbCoeffs + 1];
    x1 = new double[nbCoeffs + 1];


    for(int i = 0; i < nbCoeffs + 1; i++){
        coeffs[i] = 0;
        weights[i] = 0;
    }

    weights[0] = dataset->train.size();
    for(int i : dataset->train){
        uint8_t*  xi = x + p * i;
        for(int j = 0; j < p; j++){
            weights[xi[j] + offsets[j] + 1] += 1;
        }
    }

    stdev[0] = 1;
    x0[0] = 0;
    x1[0] = 1;
    for(int i = 1; i < nbCoeffs + 1; i++){
        double w = weights[i] / weights[0];
        double s= std::sqrt(w - w * w);
        stdev[i] = s;
        if(s > 0){
            x1[i] = (1 - w) / s;
            x0[i] = (0 - w) / s;
        } else {
            x1[i] = 0;
            x0[i] = 0;
        }
    }

    for(int i=0; i < p; i++){
        selected_features.insert(i);
    }
    for(std::string feature : config->excludedFeatures){
        int featureIdx = config->getFeatureIndex(feature);
        eraseFeature(0, featureIdx);
    }

    ypred.reserve(n);
    dppred.reserve(n);
}

ALinearRegressor::~ALinearRegressor()
{
    delete[] coeffs;
    delete[] weights;
    delete[] stdev;
    delete[] x0;
    delete[] x1;
}

int ALinearRegressor::penalizeLasso(float learning_rate, float l1){
    int nb_coeffs_non_zero = 0;
    for(int j = 0; j < nbCoeffs + 1; j++){
        float c = coeffs[j];
        if(c > l1 * learning_rate){
            coeffs[j] -= l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else if(c < -l1 * learning_rate){
            coeffs[j] += l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else {
            coeffs[j] = 0;
        }
    }
    return nb_coeffs_non_zero;
}

void ALinearRegressor::penalizeRidge(float learning_rate, float l2){
    for(int j = 1; j < nbCoeffs + 1; j++){
        coeffs[j] *= (1 - l2 * learning_rate);
    }
}

void ALinearRegressor::predict(){
    double dp0 = 0;
    for(int j = 1; j < nbCoeffs; j++){
        dp0 += x0[j] * coeffs[j];
    }
    for(int i = 0; i < n; i++){
        uint8_t* xi = x + p * i;
        double dp = coeffs[0] + dp0;
        for(int j = 0; j < p; j++){
            int k = offsets[j]+ xi[j] + 1;
            dp += (x1[k] - x0[k]) * coeffs[k];
        }
        dppred[i] = dp;
        ypred[i] = exp(dp) * exposure[i];
    }
}

double ALinearRegressor::logLikelihood(const std::vector<int> &samples){
    double ll = 0;
    if(config->loss == "gaussian"){
        for(int i : samples){
            double dp = dppred[i];
            double ei = exposure[i];
            ll += std::sqrt(ei * dp - y[i]);
        }
    } else if(config->loss == "poisson") {
        for(int i : samples){
            double dp = dppred[i];
            double ei = exposure[i];
            ll += ei * std::exp(dp) - y[i] * dp + std::log(ei);
        }
    } else if(config->loss == "gamma") {
        for(int i : samples){
            double dp = dppred[i];
            double ei = exposure[i];
            ll += y[i] / (ei * std::exp(dp)) + dp;
        }
    } else {
        throw std::invalid_argument( "Received invalid loss function." );
    }
    return ll / samples.size();
}

std::string doubleToText(const double & d)
{
    std::stringstream ss;
    ss << std::setprecision( std::numeric_limits<int>::max() );
    ss << d;
    return ss.str();
}

void ALinearRegressor::writeResults(std::string filename,
                                    std::vector<int> test){
    std::cout << "Writting results." << std::endl;
    std::ofstream resultFile;
    resultFile.open(filename.c_str(), std::ios::out);
    resultFile << "row,exposure,target,prediction" << std::endl;
    for(int i : test){
        resultFile << i << "," << exposure[i] << "," << y[i] << ","
            << ypred[i] << std::endl;
    }
    resultFile.close();

    std::ofstream coeffFile;
    coeffFile.open("data/mrh/coeffs.csv", std::ios::out);
    coeffFile << "Coeffs" << std::endl;
    for(int j=0; j < nbCoeffs + 1; j++){
        double c = stdev[j] != 0 ? coeffs[j] / stdev[j] : 0;
        coeffFile << doubleToText(c) << std::endl;
    }
    coeffFile.close();
}

double ALinearRegressor::getCoeffNorm2(int feature){
    double sc = 0;
    double sw = 0;
    for(int j = offsets[feature]; j < offsets[feature + 1] ; j++){
        double c = coeffs[j + 1];
        double w = weights[j + 1];
        sc += c * c * w;
        sw += w;
    }
    return std::sqrt(sc / sw);
}

int ALinearRegressor::getMinCoeff(std::set<int>& selected_features){
    int minidx = -1;
    double minvalue = 100000000;
    for(int i = 0; i< p; i++){
        double s = getCoeffNorm2(i);
        if(selected_features.count(i) && s < minvalue){
            minvalue = s;
            minidx = i;
        }
    }
    return minidx;
}

double ALinearRegressor::getSpread(int feature){
    double minvalue = 100000000;
    double maxvalue = 0;

    for(int j = offsets[feature]; j < offsets[feature + 1] ; j++){
        float c = std::exp(coeffs[j + 1]  / stdev[j + 1]);
        if(c < minvalue) minvalue = c;
        if(c > maxvalue) maxvalue = c;
    }

    return float(std::round((maxvalue / minvalue - 1) * 10000)) / 100;
}

void ALinearRegressor::eraseFeature(int i, int remove_feature){
    std::cout << i << " : Removing " << config->features[remove_feature]
              << " " << remove_feature << " "
              << " Norm2=" << getCoeffNorm2(remove_feature) << std::endl;
    selected_features.erase(remove_feature);
    for(int j = config->offsets[remove_feature];
        j < config->offsets[remove_feature + 1]; j++){
        coeffs[j + 1] = 0;
        x0[j + 1] = 0;
        x1[j + 1] = 0;
    }
}

void ALinearRegressor::printSelectedFeatures(){
    std::map<double, int> keep_features;
    for(auto f : selected_features){
        double v = getSpread(f);
        // FIXME : this does not work if two features have the same spread !
        keep_features[v] = f;
    }

    int i = 1;
    std::cout << "Selected Features :" <<std::endl;
    for(auto kv = keep_features.rbegin(); kv != keep_features.rend(); kv++){
        int featureIdx = kv->second ;
        std::cout << "        " << i << " : " << features[featureIdx] << " [N2="
                  << getCoeffNorm2(featureIdx) << ", Spread(100/0)="
                  << getSpread(featureIdx) << "%]" << std::endl;
        i++;
    }
}

double ALinearRegressor::getNorm2CoeffDiff(double* coeffs2){
    int size = 0;
    double s = 0;
    for(int i = 0; i < config->m + 1; i++){
        if(coeffs[i] != 0 || coeffs2[i] != 0){
            size++;
            double diff = (coeffs[i] - coeffs2[i]) / stdev[i];
            s += diff * diff ;
        }
    }
    return std::sqrt(s /  size);
}

double ALinearRegressor::rmse(const std::vector<int> &samples){
    double rmse = 0;
    double sexp = 0;
    for(int j=0; j < samples.size(); j++){
        int i = samples[j];
        double e = y[i] - ypred[i];
        rmse += e * e;
        sexp += exposure[i];
    }
    return std::sqrt(rmse/sexp);
}

double ALinearRegressor::gini(const std::vector<int> &samples){
    std::vector<size_t> idx = reverse_sort_indexes(ypred, samples);
    double exposure_sum = 0;
    double obs_sum = 0;
    double rank_obs_sum = 0;
    for(int i : idx){
        int obs = samples[i];
        double e = exposure[obs];
        exposure_sum += e;
        obs_sum += e * y[obs];
        rank_obs_sum += y[obs] * e * (exposure_sum - 0.5 * e);
    }
    return 1 - (2 / (exposure_sum * obs_sum)) * rank_obs_sum;
}

void ALinearRegressor::printResults(const std::vector<int> &train,
                                   const std::vector<int> &test){
    std::cout << "rmse(train=" << rmse(train) << ", test=" << rmse(test) << ")"
              << " | "
              << "gini(train=" << gini(train) << ", test=" << gini(test) << ")"
              << " | "
              << "ll(train=" << logLikelihood(train) << ", test="
              << logLikelihood(test) << ")"
              << std::endl;
}

std::vector<size_t> ALinearRegressor::reverse_sort_indexes(
        const std::vector<float> &v, const std::vector<int> &samples)
{
    // initialize original index locations
    std::vector<size_t> idx(samples.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
        [&v, &samples](size_t i1, size_t i2) {
            return v[samples[i1]] > v[samples[i2]];
        }
    );

  return idx;
}
