#ifndef SGDREGRESSOR_H_
#define SGDREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class SGDRegressor : public ALinearRegressor
{
public:
    SGDRegressor(Config* config, Dataset* dataset);
    virtual ~SGDRegressor();
    int getBlockSize();
    virtual void fit();

private:
    int blocksize;
    float learningRate;
    std::vector<double> update;

    double (*gradLoss)(double, double, double);
    void fitIntercept();
};

#endif  // SGDREGRESSOR_H_
