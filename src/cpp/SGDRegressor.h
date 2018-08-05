#ifndef SGDREGRESSOR_H_
#define SGDREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class SGDRegressor : public ALinearRegressor
{
private:
    int blocksize;
    float learningRate;
    std::vector<double> update;
    double (*gradLoss)(double, double, double);

public:
    SGDRegressor(Config* config, Dataset* dataset, int blocksize,
                 float learningRate);
    int getBlockSize();
    virtual void fit();


private:
    void fitIntercept();
};

#endif  // SGDREGRESSOR_H_
