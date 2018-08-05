#ifndef SGDREGRESSOR_H_
#define SGDREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class SGDRegressor : public ALinearRegressor
{
    std::vector<double> update;
    double (*gradLoss)(double, double, double);

public:
    SGDRegressor(Config* config, Dataset* dataset);
    virtual void fit(int, float, float);
    void fitIntercept();
};

#endif  // SGDREGRESSOR_H_
