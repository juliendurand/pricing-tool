#include <iostream>

#include "SGDPoissonRegressor.h"

SGDPoissonRegressor::SGDPoissonRegressor(int p, int n,
    uint8_t* x, float* y, float* exposure) : ALinearRegressor(p, n, x, y, exposure)
{
}


void SGDPoissonRegressor::fit(int i, float learning_rate)
{
    uint8_t* xi = x + p * i;

    double yi = y[i];
    double ei = exposure[i];

    double dp = coeffs[0];
    for(int j = 0; j < p; j++){
        if(xi[j] > 200){
            std::cout << "ERROR" << std::endl;
        }
        dp += coeffs[(j + 1) * 200 + xi[j]];
    }
    double y_pred = exp(dp) * ei;
    double r = learning_rate * (yi - y_pred);

    coeffs[0] += r;
    for(int j = 0; j < p ; j++){
        coeffs[(j + 1) * 200 + xi[j]] += r;
    }
}
