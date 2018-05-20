#ifndef ALINEARREGRESSOR_H_
#define ALINEARREGRESSOR_H_

#include <string>
#include <vector>
#include <set>

#include "config.h"
#include "dataset.h"

struct FeatureResult {
    int feature_idx;
    std::string feature;
    double gini;
    double coeffGini;
    double norm;
    double spread100;
    double spread95;
    double rmse;
};


class ALinearRegressor
{
public:
    Config* config;
    Dataset* dataset;
    int p;
    int n;
    uint8_t* x;
    float* y;
    float* exposure;
    double* coeffs;
    double* weights;
    double* stdev;
    double* x0;
    double* x1;
    int nbCoeffs;
    std::vector<int> offsets;
    std::vector<std::string> features;
    std::set<int> selected_features;
    std::vector<float> dppred;
    std::vector<float> ypred;
    std::vector<FeatureResult> giniPath;

    ALinearRegressor(Config* config, Dataset* dataset);
    ~ALinearRegressor();
    virtual void fit(int, float, float) = 0;
    int penalizeLasso(float learning_rate, float l1);
    void penalizeRidge(float learning_rate, float l2);
    void predict(const std::vector<int> &samples);
    double logLikelihood(const std::vector<int> &samples);
    void writeResults(std::vector<int> test);
    int getMinCoeff(std::set<int>& selected_features);
    double getCoeffNorm2(int feature);
    double getCoeffGini(int feature);
    double getSpread95(int feature);
    double getSpread100(int feature);
    void eraseFeature(int i, int feature);
    void addFeatures(const std::vector<int> &features);
    void printSelectedFeatures();
    double getNorm2CoeffDiff(double* coeffs2);
    double rmse(const std::vector<int> &samples);
    double gini(const std::vector<int> &samples);
    void printResults();
    void writeGiniPath();
    std::vector<int> getBestFeatures();

private:
    std::vector<size_t> reverse_sort_indexes(const std::vector<float> &v,
                                             float* w,
                                             const std::vector<int> &samples);

};

#endif  // ALINEARREGRESSOR_H_
