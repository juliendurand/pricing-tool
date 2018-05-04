#ifndef METRICS_H_
#define METRICS_H_

#include <stdint.h>
#include <vector>

#include "ALinearRegressor.h"
#include "config.h"


class LinearRegressionResult
{
public:
    uint8_t* x;
    float* y;
    std::vector<float> ypred;
    float* exposure;
    double* coeffs;

    LinearRegressionResult(ALinearRegressor* linearRegressor);
    double rmse(const std::vector<int> &samples);
    double gini(const std::vector<int> &samples);
    void print(const std::vector<int> &train, const std::vector<int> &test);

private:
    double area_lorentz_fast(const std::vector<int> &samples);
};

#endif  // METRICS_H_
