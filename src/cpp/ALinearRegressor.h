#ifndef ALINEARREGRESSOR_H_
#define ALINEARREGRESSOR_H_

#include <set>
#include <string>
#include <vector>

#include "Config.h"
#include "Dataset.h"


class ALinearRegressor
{
public:
    Config* config;
    Dataset* dataset;
    int p;
    int n;
    std::set<int> selected_features;

    ALinearRegressor(Config* config, Dataset* dataset);
    virtual ~ALinearRegressor();
    virtual void fit() = 0;
    std::vector<double> getCoeffs();
    void predict(const std::vector<int> &samples);
    int getMinCoeff();
    double getCoeffNorm2(int feature);
    double getCoeffGini(int feature);
    double getSpread95(int feature);
    double getSpread100(int feature);
    void eraseAllFeatures();
    void eraseFeatures(const std::vector<int> &features);
    void addFeatures(const std::vector<int> &features);
    double logLikelihood(const std::vector<int> &samples);
    double rmse(const std::vector<int> &samples);
    double gini(const std::vector<int> &samples);
    void printResults();
    void writeResults(const std::vector<int> test);

protected:
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
    std::vector<float> dppred;
    std::vector<float> ypred;
    std::vector<double> g;

    const std::vector<size_t> reverse_sort_indexes(const std::vector<float> &v,
        const float* w, const std::vector<int> &samples);
};

#endif  // ALINEARREGRESSOR_H_
