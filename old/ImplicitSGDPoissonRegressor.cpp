#include <iostream>

#include "ImplicitSGDPoissonRegressor.h"

ImplicitSGDPoissonRegressor::ImplicitSGDPoissonRegressor(int p, int n,
    uint8_t* x, float* y, float* exposure) : ALinearRegressor(p, n, x, y, exposure)
{
}

void ImplicitSGDPoissonRegressor::fit(int i, float learning_rate)
{
    uint8_t* xi = x + p * i;

    double yi = y[i];
    double ei = exposure[i];

    if(ei < 0.2){
        return;
    }
    yi /= ei;

    double dp = coeffs[0];
    for(int j = 0; j < p; j++){
        if(xi[j] > 200){
            std::cout << "ERROR" << std::endl;
        }
        dp += coeffs[(j + 1) * 200 + xi[j]];
    }
    double y_pred = exp(dp);
    double r = learning_rate * (yi - y_pred);
    double x_sq = 1 + p;  // optimization
    /*
    for(int j = 0; j < p; j++){
        std::cout << (int) xi[j] << std::endl;
        x_sq += xi[j] * xi[j];
    }
    */
    double x1 = 0, x2 = r, x3 = 0;
    double y1 = x1 - learning_rate * (yi - y_pred * exp(x_sq * x1));
    double y2 = x2 - learning_rate * (yi - y_pred * exp(x_sq * x2));
    double y3 = 0;
    for(int j=0; (j < 30) && (std::abs(y2) > 1e-12); j++){
        //std::cout << x1 << " " << x2 << " " << y1 << " " << y2 << std::endl;
        x3 = (x1 * y2 - x2 * y1) / (y2 - y1);
        y3 = x3 - learning_rate * (yi - y_pred * exp(x_sq * x3));
        x1 = x2;
        y1 = y2;
        x2 = x3;
        y2 = y3;
        //std::cout << x1 << " " << x2 << " " << y1 << " " << y2 << std::endl;
    }
    //std::cout << yi << " " << y_pred << " " << dp << " "<< x_sq << " " << r << " " << x2 << std::endl;
    coeffs[0] += x2;
    for(int j = 0; j < p ; j++){
        coeffs[(j + 1) * 200 + xi[j]] += x2;
    }
    /*for(int j = 0; j < (p+1) * 200 ; j++){
        coeffs_star[j] += coeffs[j];
    }*/
}
