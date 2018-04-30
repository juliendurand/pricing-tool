#ifndef SGDPOISSONREGRESSOR_H_
#define SGDPOISSONREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class SGDPoissonRegressor : public ALinearRegressor
{
    std::vector<double> update;

public:
    SGDPoissonRegressor(int, int, uint8_t*, float*, float*, int nbCoeffs, const std::vector<int> &offsets, std::vector<std::string> &features);
    virtual void fit(int, float);
    void filterfit(int, float, std::set<int> &feature_filters);
    void blockfit(Dataset& ds, int blocksize, float learning_rate,
                  std::set<int> &feature_filters);
    void squeezeCoeffs();
};

#endif  // SGDPOISSONREGRESSOR_H_
