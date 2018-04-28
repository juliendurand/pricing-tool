#ifndef CDPOISSONREGRESSOR_H_
#define CDPOISSONREGRESSOR_H_

#include "ALinearRegressor.h"


class CDPoissonRegressor : public ALinearRegressor
{
public:
    CDPoissonRegressor(int, int, uint8_t*, float*, float*, int nbCoeffs, const std::vector<int> &offsets, std::vector<std::string> &features);
    ~CDPoissonRegressor();

    virtual void fit(int, float);
    void blockfit(Dataset& ds, int blocksize);

    double* sy;
    double* syp;
    double* sx;
};

#endif  // CDPOISSONREGRESSOR_H_
