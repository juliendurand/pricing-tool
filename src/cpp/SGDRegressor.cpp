#include "SGDRegressor.h"

#include <iostream>


SGDRegressor::SGDRegressor(Config* config, Dataset* dataset, int blocksize,
                           float learningRate):
    ALinearRegressor(config, dataset),
    blocksize(blocksize),
    learningRate(learningRate)
{
    update.reserve(nbCoeffs + 1);
    if(config->loss == "gaussian"){
        gradLoss = [](double y, double dp, double weight){
            return y - dp * weight;
        };
        std::cout << "Using gaussian loss" << std::endl;
    } else if(config->loss == "poisson") {
        gradLoss = [](double y, double dp, double weight){
            return y - std::exp(dp) * weight;
        };
        std::cout << "Using poisson loss" << std::endl;
    } else if(config->loss == "gamma") {
        gradLoss = [](double y, double dp, double weight){
            return y / (std::exp(dp) * weight) - 1;
        };
        std::cout << "Using gamma loss" << std::endl;
    } else {
        throw std::invalid_argument("Received invalid loss function.");
    }
    fitIntercept();
}

int SGDRegressor::getBlockSize(){
    return blocksize;
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
            g[j] = 0.9 * g[j] + /*0.1 **/ grad;
            coeffs[j] += learningRate * g[j];
        }
    }
/*
    coeffs[0] += rTotal / blocksize * learning_rate;
    for(int j = 1; j < nbCoeffs + 1 ; j++){
        if(weights[j] < std::sqrt(weights[0]) / 10){
            // squeezing non significative coefficients to Zero
        //    coeffs[j] = 0; // this line is not required (just to be explicit) !
        } else {
            //coeffs[j] *= (1 - l2);
            coeffs[j] += (update[j] + rTotal * x0[j]) / blocksize * learning_rate;
        }
        //std::cout << j << ":" << coeffs[j] << ", ";
    }
    //std::cout << update[1] << " " << rTotal << ":" << coeffs[1] << ", "  << std::endl;
*/
}
