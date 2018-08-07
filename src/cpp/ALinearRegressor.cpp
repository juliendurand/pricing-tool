#include "ALinearRegressor.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>


ALinearRegressor::ALinearRegressor(Config* config, Dataset* ds):
    config(config),
    dataset(ds),
    p(config->p),
    n(config->n),
    x(ds->get_x()),
    y(ds->get_y()),
    exposure(ds->get_weight()),
    nbCoeffs(config->m),
    offsets(config->offsets),
    features(config->features)
{
    coeffs.resize(nbCoeffs + 1, 0);
    weights.resize(nbCoeffs + 1, 0);
    stdev.resize(nbCoeffs + 1, 0);
    x0.resize(nbCoeffs + 1, 0);
    x1.resize(nbCoeffs + 1, 0);
    g.resize(nbCoeffs + 1, 0);
    ypred.resize(n, 0);
    dppred.resize(n, 0);

    for(int i : dataset->getTrain()){
        weights[0] += exposure[i];
        for(int j = 0; j < p; j++){
            weights[x[p * i+ j] + offsets[j] + 1] += exposure[i];
        }
    }

    for(int i = 0; i < nbCoeffs + 1; i++){
        double w = weights[i] / weights[0];
        stdev[i] = std::sqrt(w - w * w);
    }

    x0[0] = 1;
    x1[0] = 1;

    for(int i = 0; i < p; i++){
            addFeatures({i});
    }
    for(std::string feature : config->excludedFeatures){
        int featureIdx = config->getFeatureIndex(feature);
        if(featureIdx >= 0){
            std::cout << "Exclude feature " << feature << std::endl;
            eraseFeatures({featureIdx});
        } else {
            std::cout << "WARNING : Excluded feature " << feature
                      << " can not be found." << std::endl;
        }
    }
}

ALinearRegressor::~ALinearRegressor(){
}

std::vector<double> ALinearRegressor::getCoeffs(){
    std::vector<double> results(coeffs.size(), 0);
    results[0] = coeffs[0];
    for(int i = 1; i < coeffs.size(); i++){
        if(stdev[i] != 0){
            results[i] = coeffs[i] / stdev[i];
            results[0] -= results[i] * (weights[i] / weights[0]);
        }
    }
    return results;
}

void ALinearRegressor::predict(const std::vector<int> &samples){
    double dp0 = coeffs[0];
    for(int j = 1; j < nbCoeffs; j++){
        dp0 += x0[j] * coeffs[j];
    }
    for(int i : samples){
        double dp = dp0;
        for(int j : selected_features){
            int k = offsets[j] + x[p * i + j] + 1;
            dp += (x1[k] - x0[k]) * coeffs[k];
        }
        dppred[i] = dp;
        ypred[i] = exp(dp) * exposure[i];
    }
}

int ALinearRegressor::getMinCoeff(){
    int minidx = -1;
    double minvalue = 0;
    for(int i : selected_features){
        double s = getCoeffGini(i);
        if(minidx == -1 || s < minvalue){
            minvalue = s;
            minidx = i;
        }
    }
    return minidx;
}

double ALinearRegressor::getCoeffNorm2(int feature){
    double sc = 0;
    double sw = 0;
    for(int j = offsets[feature]; j < offsets[feature + 1] ; j++){
        double c = stdev[j + 1] > 0 ? coeffs[j + 1] / stdev[j + 1] : 0;
        double w = weights[j + 1];
        sc += c * c * w;
        sw += w;
    }
    return std::sqrt(sc / sw);
}

double ALinearRegressor::getCoeffGini(int feature){
    if(feature < 0){
        return 0;
    }

    int nb_coeffs = offsets[feature + 1] - offsets[feature];
    std::vector<int> feature_idx(nb_coeffs);
    for(int i = 0; i < nb_coeffs; i++){
        feature_idx[i] = offsets[feature] + i;
    }
    std::sort(feature_idx.begin(), feature_idx.end(),
        [this](size_t i, size_t j) {
            return std::exp(this->coeffs[i + 1] / this->stdev[i + 1]) <
                   std::exp(this->coeffs[j + 1] / this->stdev[j + 1]);
        }
    );

    double g = 0;
    double sc = 0;
    double sw = 0;
    for(int i : feature_idx){
        int j = i + 1;
        double w = weights[j];
        double c = stdev[j] != 0 ? std::exp(coeffs[j]) * w : 1;
        g += w * (2 * sc + c);
        sc += c;
        sw += w;
    }
    g = 1 - g / (sc * sw);
    return g < 0.0000001 ? 0 : g;
}

double ALinearRegressor::getSpread100(int feature){
    if(feature < 0){
        return 0;
    }

    double minvalue = 100000000;
    double maxvalue = 0;

    for(int j = offsets[feature]; j < offsets[feature + 1] ; j++){
        float c = std::exp(coeffs[j + 1]  / stdev[j + 1]);
        if(c < minvalue) minvalue = c;
        if(c > maxvalue) maxvalue = c;
    }

    return float(std::round((maxvalue / minvalue - 1) * 10000)) / 100;
}

double ALinearRegressor::getSpread95(int feature){
    if(feature < 0){
        return 0;
    }
    int nb_coeffs = offsets[feature + 1] - offsets[feature];
    std::vector<int> feature_idx(nb_coeffs);
    for(int i = 0; i < nb_coeffs; i++){
        feature_idx[i] = offsets[feature] + i;
    }
    std::sort(feature_idx.begin(), feature_idx.end(),
        [this](size_t i, size_t j) {
            return std::exp(this->coeffs[i + 1] / this->stdev[i + 1])
                   < std::exp(this->coeffs[j + 1] / this->stdev[j + 1]);
        }
    );
    std::vector<double> cum_weight(nb_coeffs);
    for(int i = 0; i < nb_coeffs; i++){
        cum_weight[i] = (i > 0 ? cum_weight[i - 1] : 0) +
                        (weights[feature_idx[i] + 1] / weights[0]);
    }
    double minvalue = 0;
    double maxvalue = 0;
    for(int i = 0; i < nb_coeffs; i++){
        if(cum_weight[i] > 0.05){
            int j = feature_idx[i] + 1;
            if(stdev[j] == 0){
                continue;
            }
            minvalue = std::exp(coeffs[j]/ stdev[j]);
            break;
        }
    }
    for(int i = 0; i < nb_coeffs; i++){
        if(cum_weight[i] > 0.95){
            int j = feature_idx[i] + 1;
            if(stdev[j] == 0){
                continue;
            }
            maxvalue = std::exp(coeffs[j]/ stdev[j]);
            break;
        }
    }

    return float(std::round((maxvalue / minvalue - 1) * 10000)) / 100;
}

void ALinearRegressor::eraseAllFeatures(){
    std::vector<int> allFeatures(selected_features.begin(),
                                 selected_features.end());
    eraseFeatures(allFeatures);
}

void ALinearRegressor::eraseFeatures(const std::vector<int> &features){
    for(int f : features){
        selected_features.erase(f);
        for(int j = config->offsets[f];
            j < config->offsets[f + 1]; j++){
            coeffs[j + 1] = 0;
            x0[j + 1] = 0;
            x1[j + 1] = 0;
        }
    }
}

void ALinearRegressor::addFeatures(const std::vector<int> &features){
    for(int f : features){
        selected_features.insert(f);
        for(int j = config->offsets[f];
            j < config->offsets[f + 1]; j++){
            int i = j + 1;
            double w = weights[i] / weights[0];
            double s = stdev[i];
            if(s > 0 && (weights[i] > std::sqrt(weights[0]) / 10)){
                x1[i] = (1 - w) / s;
                x0[i] = (0 - w) / s;
            } else {
                x1[i] = 0;
                x0[i] = 0;
            }
        }
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
    std::vector<size_t> idx = reverse_sort_indexes(ypred, exposure, samples);
    double exposure_sum = 0;
    double obs_sum = 0;
    double rank_obs_sum = 0;
    for(int i : idx){
        int obs = samples[i];
        double e = exposure[obs];
        exposure_sum += e;
        obs_sum += y[obs];
        rank_obs_sum += y[obs] * (exposure_sum - 0.5 * e);
    }
    return 1 - (2 / (exposure_sum * obs_sum)) * rank_obs_sum;
}

void ALinearRegressor::printResults(){
    predict(dataset->getTrain());
    predict(dataset->getTest());
    predict(dataset->getSample());
    std::cout << "gini(train=" << gini(dataset->getTrain())
              << ", test="     << gini(dataset->getTest())
              << ", sample="     << gini(dataset->getSample()) << ")"
              << " | "
              << "ll(train=" << logLikelihood(dataset->getTrain())
              << ", test="   << logLikelihood(dataset->getTest()) << ")"
              << std::endl;
}

void ALinearRegressor::writeResults(std::vector<int> test){
    predict(test);
    std::cout << std::endl << "Saving results." << std::endl;

    std::ofstream resultFile;
    resultFile.open(config->resultPath + "results.csv", std::ios::out);
    resultFile << "row,exposure,target,prediction" << std::endl;
    for(int i : test){
        resultFile << i << "," << exposure[i] << "," << y[i] << ","
            << ypred[i] << std::endl;
    }
    resultFile.close();

    std::ofstream coeffFile;
    coeffFile.open(config->resultPath + "coeffs.csv", std::ios::out);
    coeffFile << "Coeffs" << std::endl;
    for(double c : getCoeffs()){
        coeffFile << c << std::endl;
    }
    coeffFile.close();
}

const std::vector<size_t> ALinearRegressor::reverse_sort_indexes(
        const std::vector<float>& v, const float* w,
        const std::vector<int>& samples)
{
    // initialize original index locations
    std::vector<size_t> idx(samples.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
        [&v, w, &samples](size_t j1, size_t j2) {
            int i1 = samples[j1];
            int i2 = samples[j2];
            return v[i1] / w[i1] > v[i2] / w[i2];
        }
    );

  return idx;
}
