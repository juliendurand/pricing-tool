#ifndef ALINEARREGRESSOR_H_
#define ALINEARREGRESSOR_H_

#include <random>
#include <string>
#include <vector>
#include <set>

#include "array.h"
#include "config.h"

class Dataset
{
public:
    std::vector<int> train;
    std::vector<int> test;
    std::mt19937 generator;
    std::uniform_int_distribution<std::mt19937::result_type> random;
    Array<uint8_t>* x_data;
    Array<float>* exposure_data;
    Array<float>* y_data;

    Dataset();
    Dataset(Config& config, float testPct);
    int next();
};

class ALinearRegressor
{
public:
    Config config;
    Dataset dataset;
    int p;
    int n;
    uint8_t* x;
    float* y;
    float* exposure;
    double* coeffs;
    double* weights;
    double* stdev;
    double* x1;
    double* x0;
    int nbCoeffs;
    std::vector<int> offsets;
    std::vector<std::string> features;

    ALinearRegressor(Config& config, Dataset& dataset);
    ~ALinearRegressor();
    virtual void fit(int, float) = 0;
    std::vector<double> covarianceProduct(const std::vector<int> &samples);
    int penalizeLasso(float learning_rate, float l1);
    int penalizeGroupLasso(float learning_rate, float l1);
    void penalizeRidge(float learning_rate, float l2);
    double pred(int);
    std::vector<float> predict();
    void writeResults(std::string filename , std::vector<int> test);
    int getMinCoeff(std::set<int>& selected_features);
    double getCoeffNorm2(int feature);
    double getSpread(int feature);
    void eraseFeature(int i, int feature, Config& config);
};

#endif  // ALINEARREGRESSOR_H_
