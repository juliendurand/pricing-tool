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
        gradLoss = [](double y, double dp, double weight){return y / (std::exp(dp) * 1 /*TODO weight*/) - 1;};
        std::cout << "Using gamma loss" << std::endl;
    } else {
        throw std::invalid_argument( "Received invalid loss function." );
    }
}

void SGDPoissonRegressor::fit(int i, float learning_rate)
{
    uint8_t* xi = x + p * i;

    double r = learning_rate * (y[i]- pred(i));

    coeffs[0] += r;
    for(int j = 1; j < nbCoeffs + 1; j++){
        coeffs[j] += r * x0[j];
    }
    for(int j : selected_features){
        int k = offsets[j]+ xi[j] + 1;
        coeffs[k] += r * (x1[k] - x0[k]);
    }
}

void SGDPoissonRegressor::fitIntercept(){
    double s = 0;
    for(int i : dataset->train){
        s += y[i];
    }
    coeffs[0] = std::log(s / dataset->train.size());
}

void SGDPoissonRegressor::blockfit(int blocksize, float learning_rate){
    double dp0 = coeffs[0];
    for(int j = 1; j < nbCoeffs + 1 ; j++){
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
        double r = gradLoss(y[i], dp, 1 * exposure[i]);
        rTotal += r;
        for(int j = 0; j < p ; j++){
            int k = offsets[j]+ xi[j] + 1;
            update[k] += r * (x1[k] - x0[k]);
        }
    }

    coeffs[0] += rTotal * learning_rate / blocksize;
    for(int j = 1; j < nbCoeffs + 1 ; j++){
        double w = weights[j];
        if(w < std::sqrt(weights[0]) / 30){
            // squeezing non significative coefficients to Zero
            coeffs[j] = 0; // this line is not required (just to be explicit) !
        }else{
            coeffs[j] += (update[j] + rTotal * x0[j]) * learning_rate / blocksize;
        }
    }
}

void SGDPoissonRegressor::fitGamma(int blocksize, float learning_rate){
    //double dp0 = std::log(1600); // coeffs[0];
    //coeffs[0] = std::log(1218);
    double dp0 = coeffs[0];
    for(int j = 1; j < nbCoeffs + 1 ; j++){
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
        double ypred = exp(dp); // TODO * exposure[i];
        double r = y[i] / ypred - 1;
        rTotal += r;
        for(int j = 0; j < p ; j++){
            int k = offsets[j]+ xi[j] + 1;
            update[k] += r * (x1[k] - x0[k]);
        }
    }

    //std::cout << blocksize << " " << learning_rate << " " << coeffs[0]  << " " << rTotal << std::endl;;
    coeffs[0] += rTotal * learning_rate / blocksize;
    //std::cout << blocksize << " " << learning_rate << " " << coeffs[0]  << " " << rTotal << std::endl;;
    for(int j = 1; j < nbCoeffs + 1 ; j++){
        double w = weights[j];
        if(w < std::sqrt(weights[0]) / 30){
            // squeezing non significative coefficients to Zero
            coeffs[j] = 0; // this line is not required (just to be explicit) !
        }else{
            coeffs[j] += (update[j] + rTotal * x0[j]) * learning_rate / blocksize;
        }
    }
}

