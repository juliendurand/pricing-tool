#include "ALinearRegressor.h"

#include <iomanip>
#include <iostream>
#include <sstream>


ALinearRegressor::ALinearRegressor(Config* config, Dataset* ds):
    config(config),
    dataset(ds),
    p(config->p),
    n(config->n)
{
    int nbCoeffs = config->m + 1;
    coeffs.resize(nbCoeffs, 0);
    weights.resize(nbCoeffs, 0);
    stdev.resize(nbCoeffs, 0);
    x0.resize(nbCoeffs, 0);
    x1.resize(nbCoeffs, 0);
    g.resize(nbCoeffs, 0);

    uint8_t* x = dataset->get_x();
    float* weight = dataset->get_weight();

    for(int i : dataset->getTrain()){
        weights[0] += weight[i];
        for(int j = 0; j < p; j++){
            weights[x[p * i + j] + config->offsets[j] + 1] += weight[i];
        }
    }

    for(int i = 0; i < nbCoeffs; i++){
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

std::unique_ptr<Coefficients> ALinearRegressor::getCoeffs(){
    std::vector<double> results(coeffs.size(), 0);
    results[0] = coeffs[0];
    for(int i = 1; i < coeffs.size(); i++){
        if(stdev[i] != 0){
            results[i] = coeffs[i] / stdev[i];
            results[0] -= results[i] * (weights[i] / weights[0]);
        }
    }
    return std::unique_ptr<Coefficients>(new Coefficients(config, results,
                                                          weights,
                                                          selected_features));
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
