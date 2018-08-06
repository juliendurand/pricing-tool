#ifndef ALINEARREGRESSOR_H_
#define ALINEARREGRESSOR_H_

#include <string>
#include <vector>
#include <set>

#include "Config.h"
#include "Dataset.h"


struct FeatureResult {
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


class ALinearRegressor
{
public:
    Config* config;
    Dataset* dataset;
    int p;
    int n;
    const uint8_t* x;
    const float* y;
    const float* exposure;
    int nbCoeffs;
    std::vector<double> coeffs;
    std::vector<double> weights;
    std::vector<double> stdev;
    std::vector<double> x0;
    std::vector<double> x1;
    std::vector<int> offsets;
    std::vector<std::string> features;
    std::set<int> selected_features;
    std::vector<float> dppred;
    std::vector<float> ypred;
    std::vector<FeatureResult> giniPath;
    std::vector<double> g;

    ALinearRegressor(Config* config, Dataset* dataset);
    virtual ~ALinearRegressor();
    virtual void fit() = 0;
    void predict(const std::vector<int> &samples);
    int getMinCoeff(const std::set<int>& selected_features);
    double getCoeffNorm2(int feature);
    double getCoeffGini(int feature);
    double getSpread95(int feature);
    double getSpread100(int feature);
    void storeFeatureInGiniPath(int f);
    void eraseAllFeatures();
    void eraseFeatures(const std::vector<int> &features);
    void addFeatures(const std::vector<int> &features);
    void printSelectedFeatures();
    double logLikelihood(const std::vector<int> &samples);
    double rmse(const std::vector<int> &samples);
    double gini(const std::vector<int> &samples);
    void printResults();
    void sortFeatures();
    void sortFeatures(int maxNbFeatures);
    const std::vector<int> getBestFeatures(int maxNbFeatures, double treshold);
    void writeResults(const std::vector<int> test);

private:
    const std::vector<size_t> reverse_sort_indexes(const std::vector<float> &v,
        const float* w, const std::vector<int> &samples);
};

#endif  // ALINEARREGRESSOR_H_
