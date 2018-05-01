#ifndef SGDPOISSONREGRESSOR_H_
#define SGDPOISSONREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class SGDPoissonRegressor : public ALinearRegressor
{
    std::vector<double> update;

public:
    SGDPoissonRegressor(Config& config, Dataset& dataset);
    virtual void fit(int, float);
    void filterfit(int, float, std::set<int> &feature_filters);
    void blockfit(Dataset& ds, int blocksize, float learning_rate,
                  std::set<int> &feature_filters);
    void squeezeCoeffs();
};

#endif  // SGDPOISSONREGRESSOR_H_
