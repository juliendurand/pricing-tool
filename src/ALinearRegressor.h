#ifndef ALINEARREGRESSOR_H_
#define ALINEARREGRESSOR_H_

#include <random>
#include <string>
#include <vector>

class Dataset
{
public:
    std::vector<int> train;
    std::vector<int> test;
    std::mt19937 generator;
    std::uniform_int_distribution<std::mt19937::result_type> random;

    Dataset(int size, float testPct);
    int next();
};

class ALinearRegressor
{
public:
    int p;
    int n;
    uint8_t* x;
    float* y;
    float* exposure;
    float* coeffs;
    float* coeffs_star;


    ALinearRegressor(int, int, uint8_t*, float*, float*);
    ~ALinearRegressor();
    virtual void fit(int, float) = 0;
    std::vector<double> covarianceProduct(const std::vector<int> &samples);
    int penalizeLasso(float learning_rate, float l1);
    int penalizeGroupLasso(float learning_rate, float l1);
    void penalizeRidge(float learning_rate, float l2);
    double pred(int);
    std::vector<float> predict();
    void writeResults(std::string filename , std::vector<int> test);
    void printGroupedCoeffN2();
};

#endif  // ALINEARREGRESSOR_H_
