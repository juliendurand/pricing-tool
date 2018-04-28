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
    std::vector<float> ypred;
    float* exposure;
    double* coeffs;
    int* samples;


    LinearRegressionResult(int p, int n, uint8_t* x, float* y, const std::vector<float> &ypred,
                           float* exposure, double* coeffs);
    double rmse(const std::vector<int> &samples);
    double gini(const std::vector<int> &samples);

private:
    double area_lorentz_fast(const std::vector<int> &samples);
};

#endif  // METRICS_H_
