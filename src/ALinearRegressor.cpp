#include <algorithm>
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
 #include "ALinearRegressor.h"


Dataset::Dataset(int size, float testPct){
    std::vector<int> index(size);
    std::iota (std::begin(index), std::end(index), 0);
    std::random_device rd;
    generator = std::mt19937(rd());
    std::shuffle(index.begin(), index.end(), generator);
    std::size_t const testSize = static_cast<std::size_t>(size * testPct);
    train = std::vector<int>(index.begin(), index.end() - testSize);
    std::sort(train.begin(), train.end());
    test = std::vector<int>(index.end() - testSize, index.end());
    std::sort(test.begin(), test.end());
    random = std::uniform_int_distribution<std::mt19937::result_type>(0, train.size());
}

int Dataset::next(){
    return train[random(generator)];
}

ALinearRegressor::ALinearRegressor(int p, int n,
    uint8_t* x, float* y, float* exposure) :
    p(p), n(n), x(x), y(y), exposure(exposure)
{
    coeffs = new float[(p + 1) * 200];
    //coeffs_star = new float[(p + 1) * 200];
}

ALinearRegressor::~ALinearRegressor()
{
    delete[] coeffs;
    //delete[] coeffs_star;
}

std::vector<double> ALinearRegressor::covarianceProduct(const std::vector<int> &samples){
    std::vector<double> s(200);
    for(int i : samples){
        uint8_t* xi = x + p * i;
        double e = exposure[i];
        double v = e * e;
        for(int k = 0; k < p; k++){
            int pos = k * 200 * 200 + xi[k];
            for(int j = k; j < p; j++){
                s[xi[j]] += v;
            }
        }
    }
    return s;
}

int ALinearRegressor::penalizeLasso(float learning_rate, float l1){
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

int ALinearRegressor::penalizeGroupLasso(float learning_rate, float l1){
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

void ALinearRegressor::penalizeRidge(float learning_rate, float l2){
    for(int j = 0; j < p * 200 ; j++){
        coeffs[200 + j] *= (1 - l2 * learning_rate);
    }
}

double ALinearRegressor::pred(int i){
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

std::vector<float> ALinearRegressor::predict(){
    std::vector<float> ypred(n);
    for(int i = 0; i < n; i++){
        ypred[i] = pred(i);
    }
    return ypred;
}

void ALinearRegressor::writeResults(std::string filename, std::vector<int> test){
    std::ofstream resultFile;
    resultFile.open(filename.c_str(), std::ios::out);
    resultFile << "exposure" << "," << "target" << "," << "prediction" << std::endl;
    for(int i : test){
        pred(i);
        resultFile << exposure[i] << "," << y[i] << "," << pred(i) << std::endl;
    }
    resultFile.close();
}

void ALinearRegressor::printGroupedCoeffN2(){
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
