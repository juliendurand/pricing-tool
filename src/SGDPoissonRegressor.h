#ifndef SGDPOISSONREGRESSOR_H_
#define SGDPOISSONREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class SGDPoissonRegressor : public ALinearRegressor
{
    std::vector<double> update;

public:
    SGDPoissonRegressor(Config* config, Dataset* dataset);
    virtual void fit(int, float);
    void fitGamma(int, float);
    void blockfit(int blocksize, float learning_rate);
};

#endif  // SGDPOISSONREGRESSOR_H_
