#ifndef IMPLICITSGDPOISSONREGRESSOR_H_
#define IMPLICITSGDPOISSONREGRESSOR_H_

#include "ALinearRegressor.h"


class ImplicitSGDPoissonRegressor : public ALinearRegressor
{
public:
    ImplicitSGDPoissonRegressor(int, int, uint8_t*, float*, float*);
    virtual void fit(int, float);
};

#endif  // IMPLICITSGDPOISSONREGRESSOR_H_
