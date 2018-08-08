#ifndef FEATURESELECTOR_H_
#define FEATURESELECTOR_H_

#include "SGDRegressor.h"


struct FeatureResult
{
    int feature_idx;
    std::string feature;
    double gini;
    double coeffGini;
    double norm;
    double spread100;
    double spread95;
    double rmse;
    double diffGini;
};

class FeatureSelector
{
public:
    FeatureSelector(SGDRegressor* model);
    void fit();
    void printSelectedFeatures();
    void writeResults();

private:
    SGDRegressor* model;
    std::vector<FeatureResult> giniPath;

    void backwardStepwise(long& i);
    void forwardStepwise(long& i, int maxNbFeatures);
    void storeFeatureInGiniPath(int f);
    void sortFeatures();
    void sortFeatures(int maxNbFeatures);
    const std::vector<int> getBestFeatures(int maxNbFeatures, double treshold);
};

#endif  // FEATURESELECTOR_H_
