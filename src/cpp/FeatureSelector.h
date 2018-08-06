#ifndef FEATURESELECTOR_H_
#define FEATURESELECTOR_H_

#include "SGDRegressor.h"


class FeatureSelector
{
private:
    SGDRegressor* model;

public:
    FeatureSelector(SGDRegressor* model);
    void backwardStepwise(long& i);
    void forwardStepwise(long& i, int maxNbFeatures);
    void fit();
    void printSelectedFeatures();
};

#endif  // FEATURESELECTOR_H_
