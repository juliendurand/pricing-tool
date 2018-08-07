#ifndef ALINEARREGRESSOR_H_
#define ALINEARREGRESSOR_H_

#include <set>
#include <string>
#include <vector>

#include "Config.h"
#include "Dataset.h"


class ModelResult
{
public:
    std::vector<int> id;
    std::vector<double> y;
    std::vector<double> y_pred;
    std::vector<double> weights;
    std::vector<double> dp; // dot product of coefficients with observations

    ModelResult(int size, const std::string loss);
    void setObservation(int position, int id, double y, double y_pred,
        double weight, double dp);
    double logLikelihood();
    double rmse();
    double gini();

private:
    const std::string loss;
    const std::vector<size_t> reverse_sort_indexes(
        const std::vector<double>& v, const std::vector<double>& w);
};


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
    std::unique_ptr<ModelResult> predict(const std::vector<int> &samples);
    int getMinCoeff();
    double getCoeffNorm2(int feature);
    double getCoeffGini(int feature);
    double getSpread95(int feature);
    double getSpread100(int feature);
    void eraseAllFeatures();
    void eraseFeatures(const std::vector<int> &features);
    void addFeatures(const std::vector<int> &features);
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
