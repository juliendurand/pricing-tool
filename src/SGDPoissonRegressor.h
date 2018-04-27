#ifndef SGDPOISSONREGRESSOR_H_
#define SGDPOISSONREGRESSOR_H_

#include "ALinearRegressor.h"


class SGDPoissonRegressor : public ALinearRegressor
{
public:
    SGDPoissonRegressor(int, int, uint8_t*, float*, float*);
    virtual void fit(int, float);
};

#endif  // SGDPOISSONREGRESSOR_H_
