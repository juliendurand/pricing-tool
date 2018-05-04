#ifndef CDPOISSONREGRESSOR_H_
#define CDPOISSONREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class CDPoissonRegressor : public ALinearRegressor
{

public:
    CDPoissonRegressor(Config* config, Dataset* dataset);
    virtual void fit(int, float);
    void cacheColumn(int m);

    double ** columns;
};

#endif  // CDPOISSONREGRESSOR_H_
