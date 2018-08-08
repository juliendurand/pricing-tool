#ifndef FEATURESELECTOR_H_
#define FEATURESELECTOR_H_

#include "SGDRegressor.h"

// Container for feature related metrics.
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


// Greedy feature selection process for glm models. This is based on backward
// and forward stepwise. This process is empirical and *MIGHT NOt* be suited
// for all use cases.
//
// Params :
//      - model : glm regression model serving as a basis for the feature
//                selection process.
//
// Usage : use with caution as this process may not be suited to all usages.

class FeatureSelector
{
public:
    FeatureSelector(SGDRegressor* model);
    void fit();
    void printSelectedFeatures();
    void saveResults();

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
