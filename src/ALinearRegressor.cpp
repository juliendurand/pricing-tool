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
    uint8_t* x, float* y, float* exposure, int nbCoeffs, const std::vector<int> &offsets, std::vector<std::string> &features) :
    p(p), n(n), x(x), y(y), exposure(exposure), nbCoeffs(nbCoeffs), offsets(offsets), features(features)
{
    coeffs = new double[nbCoeffs + 1];
    for(int i = 0; i < nbCoeffs; i++){
        coeffs[i] = 0;
    }
}

ALinearRegressor::~ALinearRegressor()
{
    delete[] coeffs;
}

std::vector<double> ALinearRegressor::covarianceProduct(const std::vector<int> &samples){
    std::vector<double> s(200);
    for(int i : samples){
        uint8_t* xi = x + p * i;
        double e = exposure[i];
        double v = e * e;
        for(int k = 0; k < p; k++){
            for(int j = k; j < p; j++){
                s[xi[j]] += v;
            }
        }
    }
    return s;
}

int ALinearRegressor::penalizeLasso(float learning_rate, float l1){
    int nb_coeffs_non_zero = 0;
    for(int j = 0; j < nbCoeffs + 1; j++){
        float c = coeffs[j];
        if(c > l1 * learning_rate){
            coeffs[j] -= l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else if(c < -l1 * learning_rate){
            coeffs[j] += l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else {
            coeffs[j] = 0;
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
    for(int j = 1; j < nbCoeffs + 1; j++){
        coeffs[j] *= (1 - l2 * learning_rate);
    }
}

double ALinearRegressor::pred(int i){
    uint8_t* xi = x + p * i;
    double dp = coeffs[0];
    for(int j = 0; j < p; j++){
        dp += coeffs[offsets[j]+ xi[j] + 1];
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
    resultFile << "row,exposure,target,prediction" << std::endl;
    for(int i : test){
        resultFile << i << ", " << exposure[i] << "," << y[i] << "," << pred(i) << std::endl;
    }
    resultFile.close();
}

double ALinearRegressor::getCoeffNorm2(int feature){
    double s = 0;
    for(int j = offsets[feature]; j < offsets[feature + 1] ; j++){
        float c = coeffs[j + 1];
        s += c * c;
    }
    int length = offsets[feature + 1] - offsets[feature];
    return std::sqrt(s / length);
}

int ALinearRegressor::getMinCoeff(std::set<int>& selected_features){
    int minidx = -1;
    double minvalue = 100000000;
    for(int i = 0; i< p; i++){
        double s = getCoeffNorm2(i);
        if(selected_features.count(i) && s < minvalue){
            minvalue = s;
            minidx = i;
        }
    }
    return minidx;
}

double ALinearRegressor::getSpread(int feature){
    double minvalue = 100000000;
    double maxvalue = 0;

    for(int j = offsets[feature]; j < offsets[feature + 1] ; j++){
        float c = std::exp(coeffs[j + 1]);
        if(c < minvalue) minvalue = c;
        if(c > maxvalue) maxvalue = c;
    }

    return (maxvalue / minvalue - 1) * 100;
}
