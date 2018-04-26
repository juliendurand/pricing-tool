#ifndef METRICS_H_
#define METRICS_H_

#include <stdint.h>
#include <vector>


class LinearRegressionResult
{
public:
    int p;
    int n;
    int s;
    uint8_t* x;
    float* y;
    float* ypred;
    float* exposure;
    float* coeffs;
    int* samples;


    LinearRegressionResult(int p, int n, uint8_t* x, float* y, float* ypred,
                           float* exposure, float* coeffs);
    double rmse(int nbSamples, std::vector<int> samples);
    double gini(int nbSamples, std::vector<int> samples);
};

#endif  // METRICS_H_
