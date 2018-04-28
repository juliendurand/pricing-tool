#include <iostream>

#include "SGDPoissonRegressor.h"

SGDPoissonRegressor::SGDPoissonRegressor(int p, int n,
    uint8_t* x, float* y, float* exposure, int nbCoeffs,
    const std::vector<int> &offsets, std::vector<std::string> &features) : ALinearRegressor(p, n, x, y, exposure, nbCoeffs, offsets, features)
{
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
    for(int j : feature_filters){
        coeffs[offsets[j]+ xi[j] + 1] += r;
    }
}

void SGDPoissonRegressor::blockfit(Dataset &ds, int blocksize, float learning_rate){
    double* update = new double[nbCoeffs + 1];
    for(int j = 0; j < nbCoeffs + 1 ; j++){
        update[j] = 0;
    }

    for(int k = 0; k < blocksize; k++){
        int i = ds.next();

        uint8_t* xi = x + p * i;

        double r = (y[i]- pred(i));
        update[0] += r;
        for(int j = 0; j < p ; j++){
            update[offsets[j]+ xi[j] + 1] += r;
        }
    }


    for(int j = 0; j < nbCoeffs + 1 ; j++){
        coeffs[j] += update[j] * learning_rate / blocksize;
        //if(j<10)
        //std::cout << update[j] / blocksize <<  std::endl;
    }

    delete[] update;
}
