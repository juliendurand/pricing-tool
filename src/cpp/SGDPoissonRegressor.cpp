#include <iostream>
#include <stdexcept>

#include "SGDPoissonRegressor.h"


SGDPoissonRegressor::SGDPoissonRegressor(Config* config, Dataset* dataset):
    ALinearRegressor(config, dataset)
{
    update.reserve(nbCoeffs + 1);
    if(config->loss == "gaussian"){
        gradLoss = [](double y, double dp, double weight){return y - dp * weight;};
        std::cout << "Using gaussian loss" << std::endl;
    } else if(config->loss == "poisson") {
        gradLoss = [](double y, double dp, double weight){return y - std::exp(dp) * weight;};
        std::cout << "Using poisson loss" << std::endl;
    } else if(config->loss == "gamma") {
        gradLoss = [](double y, double dp, double weight){return y / (std::exp(dp) * weight) - 1;};
        fitIntercept();
        std::cout << "Using gamma loss" << std::endl;
    } else {
        throw std::invalid_argument( "Received invalid loss function." );
    }
}

void SGDPoissonRegressor::fitIntercept(){
    double s = 0;
    double w = 0;
    for(int i : dataset->train){
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

void SGDPoissonRegressor::fit(int blocksize, float learning_rate, float l2){
    double dp0 = coeffs[0];
    for(int j = 1; j < nbCoeffs + 1; j++){
        dp0 += x0[j] * coeffs[j];
        update[j] = 0;
    }

    double rTotal = 0;
    for(int k = 0; k < blocksize; k++){
        int i = dataset->next();
        uint8_t* xi = x + p * i;

        double dp = dp0;
        for(int j : selected_features){
            int k = offsets[j]+ xi[j] + 1;
            dp += (x1[k] - x0[k]) * coeffs[k];
        }
        double r = gradLoss(y[i], dp, exposure[i]);
        rTotal += r;
        for(int j = 0; j < p ; j++){
            int k = offsets[j]+ xi[j] + 1;
            update[k] += r * (x1[k] - x0[k]);
        }
    }

    update[0] = rTotal;
    //double sg2 = 0;
    for(int j = 0; j < nbCoeffs + 1; j++){
        if(x1[j] != 0){
            double grad = (update[j] + rTotal * x0[j]) / blocksize;
            g[j] = 0.9 * g[j] + /*0.1 **/ grad;
            coeffs[j] += learning_rate * g[j];
            //g2[j] = 0.999 * g2[j] +0.001 * grad * grad;
            //sg2 += g2[j];
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
