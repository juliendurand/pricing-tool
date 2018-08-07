#ifndef SGDREGRESSOR_H_
#define SGDREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class SGDRegressor : public ALinearRegressor
{
public:
    SGDRegressor(Config* config, Dataset* dataset);
    virtual ~SGDRegressor();
    virtual void fit();
    void fitEpoch(long& i, float nb_epoch);
    void fitUntilConvergence(long& i, int precision, float stopCriterion);

private:
    int blocksize;
    float learningRate;
    std::vector<double> update;

    double (*gradLoss)(double, double, double);
    void fitIntercept();
};

#endif  // SGDREGRESSOR_H_
