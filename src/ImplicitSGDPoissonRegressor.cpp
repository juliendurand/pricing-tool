#include <cinttypes>
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>


 #include "array.h"
 #include "ImplicitSGDPoissonRegressor.h"


ImplicitSGDPoissonRegressor::ImplicitSGDPoissonRegressor(int p, int n,
    uint8_t* x, float* y, float* exposure) :
    p(p), n(n), x(x), y(y), exposure(exposure)
{
    coeffs = new float[(p + 1) * 200];
    //coeffs_star = new float[(p + 1) * 200];
}

ImplicitSGDPoissonRegressor::~ImplicitSGDPoissonRegressor()
{
    delete[] coeffs;
    //delete[] coeffs_star;
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

void ImplicitSGDPoissonRegressor::fit_explicit(int i, float learning_rate)
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

int ImplicitSGDPoissonRegressor::penalizeLasso(float learning_rate, float l1){
    int nb_coeffs_non_zero = 0;
    for(int j = 0; j < p * 200 ; j++){
        float c = coeffs[200 + j];
        if(c > l1 * learning_rate){
            coeffs[200 + j] -= l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else if(c < -l1 * learning_rate){
            coeffs[200 + j] += l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else {
            coeffs[200 + j] = 0;
        }
    }
    return nb_coeffs_non_zero;
}

int ImplicitSGDPoissonRegressor::penalizeGroupLasso(float learning_rate, float l1){
    int nb_coeffs_non_zero = 0;
    for(int i = 0; i< p; i++){
        float s = 0;
        for(int j = 0; j < 200 ; j++){
            float c = coeffs[(i + 1) *200 + j];
            s += c * c;
        }
        s = std::sqrt(s);
        if(s > l1 * learning_rate){
            for(int j = 0; j < 200 ; j++){
                coeffs[(i + 1) *200 + j] -= l1 * learning_rate * coeffs[(i + 1) *200 + j] / s;
            }
            nb_coeffs_non_zero++;
        } else {
            for(int j = 0; j < 200 ; j++){
                coeffs[(i + 1) *200 + j] = 0;
            }
        }
    }
    return nb_coeffs_non_zero;
}

void ImplicitSGDPoissonRegressor::penalizeRidge(float learning_rate, float l2){
    for(int j = 0; j < p * 200 ; j++){
        coeffs[200 + j] *= (1 - l2 * learning_rate);
    }
}

double ImplicitSGDPoissonRegressor::pred(int i){
    uint8_t* xi = x + p * i;
    double dp = coeffs[0];
    for(int j = 0; j < p; j++){
        if(xi[j] > 200){
            std::cout << "ERROR" << std::endl;
        }
        dp += coeffs[(j + 1) * 200 + xi[j]];
    }
    return exp(dp) * exposure[i];
}

std::vector<float> ImplicitSGDPoissonRegressor::predict(){
    std::vector<float> ypred(n);
    for(int i = 0; i < n; i++){
        ypred[i] = pred(i);
    }
    return ypred;
}

void ImplicitSGDPoissonRegressor::writeResults(std::string filename){
    std::ofstream resultFile;
    resultFile.open(filename.c_str(), std::ios::out);
    resultFile << "exposure" << "," << "target" << "," << "prediction" << std::endl;
    for(int i = 0; i < 1000000; i++){
        pred(i);
        resultFile << exposure[i] << "," << y[i] << "," << pred(i) << std::endl;
    }
    resultFile.close();
}

void ImplicitSGDPoissonRegressor::printGroupedCoeffN2(){
    for(int i = 0; i< p; i++){
        float s = 0;
        for(int j = 0; j < 200 ; j++){
            float c = coeffs[(i + 1) *200 + j];
            s += c * c;
        }
        std::cout << i << ",:" << std::sqrt(s) << "," << std::endl;
    }
    std::cout << std::endl;
}
