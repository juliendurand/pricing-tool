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


Dataset::Dataset(){
}

Dataset::Dataset(Config& config, float testPct){
    std::vector<int> index(config.n);
    std::iota (std::begin(index), std::end(index), 0);
    std::random_device rd;
    generator = std::mt19937(rd());
    std::shuffle(index.begin(), index.end(), generator);
    std::size_t const testSize = static_cast<std::size_t>(config.n * testPct);
    train = std::vector<int>(index.begin(), index.end() - testSize);
    std::sort(train.begin(), train.end());
    test = std::vector<int>(index.end() - testSize, index.end());
    std::sort(test.begin(), test.end());
    random = std::uniform_int_distribution<std::mt19937::result_type>(0, train.size());

    x_data = new Array<uint8_t>(config.getFeatureFilename(), config.p, config.n);
    exposure_data = new Array<float>(config.getExposureFilename(), 1, config.n * 4);
    y_data = new Array<float>(config.getTargetFilename(), 1, config.n * 4);
}

int Dataset::next(){
    return train[random(generator)];
}

ALinearRegressor::ALinearRegressor(Config& config, Dataset& dataset)
{

    this->config = config;
    n = config.n;
    p = config.p;
    nbCoeffs = config.m;
    offsets = config.offsets;
    features = config.features;
    x = dataset.x_data->getData();
    exposure = dataset.exposure_data->getData();
    y = dataset.y_data->getData();

    coeffs = new double[nbCoeffs + 1];
    for(int i = 0; i < nbCoeffs; i++){
        coeffs[i] = 0;
    }

    weights = new double[nbCoeffs + 1];
    stdev = new double[nbCoeffs + 1];
    x1 = new double[nbCoeffs + 1];
    x0 = new double[nbCoeffs + 1];
    for(int i = 0; i < nbCoeffs; i++){
        weights[i] = 0;
    }
    // should be calculated only on training set !!!
    for(int i = 0; i < n; i++){
        uint8_t*  xi = x + p * i;
        for(int j = 0; j < p; j++){
            weights[xi[j] + offsets[j] + 1] += 1;
        }
    }
    stdev[0] = 1;
    for(int i = 1; i < nbCoeffs; i++){
        double w = weights[i] / n;
        double s= std::sqrt(w - w * w);
        stdev[i] = s;
        x1[i] = (1 - w) / s;
        x0[i] = (0 - w) / s;
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
    for(int j = 1; j < nbCoeffs; j++){
        dp += x0[j] * coeffs[j];
    }
    for(int j = 0; j < p; j++){
        int k = offsets[j]+ xi[j] + 1;
        dp += (x1[k] - x0[k]) * coeffs[k];
    }
    return exp(dp) * exposure[i];
}

std::vector<float> ALinearRegressor::predict(){
    std::vector<float> ypred(n);
    double dp0 = 0;
    for(int j = 1; j < nbCoeffs; j++){
        dp0 += x0[j] * coeffs[j];
    }
    for(int i = 0; i < n; i++){
        uint8_t* xi = x + p * i;
        double dp = coeffs[0] + dp0;
        for(int j = 0; j < p; j++){
            int k = offsets[j]+ xi[j] + 1;
            dp += (x1[k] - x0[k]) * coeffs[k];
        }
        ypred[i] = exp(dp) * exposure[i];
        //ypred[i] = pred(i);
    }
    return ypred;
}

#include <iomanip>
#include <sstream>

std::string doubleToText(const double & d)
{
    std::stringstream ss;
    //ss << std::setprecision( std::numeric_limits<double>::digits10+2);
    ss << std::setprecision( std::numeric_limits<int>::max() );
    ss << d;
    return ss.str();
}

void ALinearRegressor::writeResults(std::string filename, std::vector<int> test){
    std::ofstream resultFile;
    resultFile.open(filename.c_str(), std::ios::out);
    resultFile << "row,exposure,target,prediction" << std::endl;
    for(int i : test){
        resultFile << i << ", " << exposure[i] << "," << y[i] << "," << pred(i) << std::endl;
    }
    resultFile.close();

    std::ofstream coeffFile;
    coeffFile.open("data/mrh/coeffs.csv", std::ios::out);
    coeffFile << "Coeffs" << std::endl;
    for(int j=0; j < nbCoeffs + 1; j++){
        double c = coeffs[j] / stdev[j];
        coeffFile << doubleToText(c) << std::endl;
    }
    coeffFile.close();
}

double ALinearRegressor::getCoeffNorm2(int feature){
    double sc = 0;
    double sw = 0;
    for(int j = offsets[feature]; j < offsets[feature + 1] ; j++){
        double c = coeffs[j + 1];
        double w = weights[j + 1];
        sc += c * c * w;
        sw += weights[j + 1];
    }
    return std::sqrt(sc / sw);
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
        float c = std::exp(coeffs[j + 1]  / stdev[j + 1]);
        if(c < minvalue) minvalue = c;
        if(c > maxvalue) maxvalue = c;
    }

    return (maxvalue / minvalue - 1) * 100;
}

void ALinearRegressor::eraseFeature(int i, int remove_feature, Config& config){
    std::cout << i << " : Removing " << config.features[remove_feature] << " Norm2=" << getCoeffNorm2(remove_feature) << std::endl;
    for(int j = config.offsets[remove_feature]; j < config.offsets[remove_feature + 1]; j++){
        coeffs[j + 1] = 0;
        x0[j + 1] = 0;
        x1[j + 1] = 0;
    }
}
