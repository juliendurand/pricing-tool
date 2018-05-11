#ifndef CDPOISSONREGRESSOR_H_
#define CDPOISSONREGRESSOR_H_

#include <vector>

#include "ALinearRegressor.h"


class CDPoissonRegressor : public ALinearRegressor
{

public:
    CDPoissonRegressor(Config* config, Dataset* dataset);
    virtual void fit(int, float);
    uint8_t* cacheColumn(int m);

    uint8_t** columns;
};

#endif  // CDPOISSONREGRESSOR_H_
