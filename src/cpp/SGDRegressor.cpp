#include "SGDRegressor.h"

#include <iostream>


SGDRegressor::SGDRegressor(Config* config, Dataset* dataset):
    ALinearRegressor(config, dataset),
    blocksize(200),
    learningRate(0.0001)
{
    update.resize(nbCoeffs + 1, 0);
    selectGradLoss(config->loss);
    fitIntercept();
}

SGDRegressor::~SGDRegressor(){
}

void SGDRegressor::selectGradLoss(const std::string loss){
    if(loss == "gaussian"){
        gradLoss = [](double y, double dp, double weight){
            return y - dp * weight;
        };
        std::cout << "Using gaussian loss" << std::endl;
    } else if(loss == "poisson") {
        gradLoss = [](double y, double dp, double weight){
            return y - std::exp(dp) * weight;
        };
        std::cout << "Using poisson loss" << std::endl;
    } else if(loss == "gamma") {
        gradLoss = [](double y, double dp, double weight){
            return y / (std::exp(dp) * weight) - 1;
        };
        std::cout << "Using gamma loss" << std::endl;
    } else {
        throw std::invalid_argument("Received invalid loss function.");
    }
}

void SGDRegressor::fitIntercept(){
    // This calculation only works if :
    //   - all coefficients are set to 0 ;
    //   - or the features have *all* been normalized.
    double s = 0;
    double w = 0;
    for(int i : dataset->getTrain()){
        s += y[i];
        w += exposure[i];
    }

    if(config->loss == "gaussian"){
        coeffs[0] = s / w;
    } else if(config->loss == "poisson") {
        coeffs[0] = std::log(s / w);
    } else if(config->loss == "gamma") {
        coeffs[0] = std::log(s / w);
    } else {
        throw std::invalid_argument( "Received invalid loss function." );
    }
}

void SGDRegressor::fit(){
    double dp0 = coeffs[0];
    for(int j = 1; j < nbCoeffs + 1; j++){
        dp0 += x0[j] * coeffs[j];
        update[j] = 0;
    }

    double rTotal = 0;
    for(int k = 0; k < blocksize; k++){
        int i = dataset->next();

        double dp = dp0;
        for(int j : selected_features){
            int k = offsets[j]+ x[p * i + j] + 1;
            dp += (x1[k] - x0[k]) * coeffs[k];
        }
        double r = gradLoss(y[i], dp, exposure[i]);
        rTotal += r;
        for(int j = 0; j < p ; j++){
            int k = offsets[j]+ x[p * i + j] + 1;
            update[k] += r * (x1[k] - x0[k]);
        }
    }

    update[0] = rTotal;
    for(int j = 0; j < nbCoeffs + 1; j++){
        if(x1[j] != 0){
            double grad = (update[j] + rTotal * x0[j]) / blocksize;
            g[j] = 0.9 * g[j] + grad;
            coeffs[j] += learningRate * g[j];
        }
    }
}

void SGDRegressor::fitEpoch(long& i, float nb_epoch){
    int epoch = dataset->getSize() / blocksize;
    int nb_blocks = nb_epoch * epoch;
    for(int j=0; j < nb_blocks; ++j){
        fit();
        ++i;
    }
}

void SGDRegressor::fitUntilConvergence(long& i, int precision,
                      float stopCriterion){
    double minll = 1e30;
    int nbIterationsSinceMinimum = 0;
    int epoch = dataset->getSize() / blocksize;
    for(; nbIterationsSinceMinimum < precision; i++){
        fit();
        if(i % epoch == 0){
            std::cout << i * blocksize << "th iteration : "
                      << " minll " << minll
                      << " iteration since min " << nbIterationsSinceMinimum
                      << std::endl;
            printResults();
            predict(dataset->getTrain());
            double ll = logLikelihood(dataset->getTrain());
            if(ll < minll - stopCriterion) {
                minll = ll;
                nbIterationsSinceMinimum = 0;
            } else {
                nbIterationsSinceMinimum++;
            }
        }
    }
}
