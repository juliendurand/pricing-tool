#ifndef SGDPOISSONREGRESSOR_H_
#define SGDPOISSONREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class SGDPoissonRegressor : public ALinearRegressor
{
    std::vector<double> update;
    double (*gradLoss)(double, double, double);

public:
    SGDPoissonRegressor(Config* config, Dataset* dataset);
    virtual void fit(int, float);
    void fitIntercept();
};

#endif  // SGDPOISSONREGRESSOR_H_
