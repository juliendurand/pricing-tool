#ifndef ALINEARREGRESSOR_H_
#define ALINEARREGRESSOR_H_

#include <string>
#include <vector>
#include <set>

#include "config.h"
#include "dataset.h"


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
    std::vector< std::pair<std::string, float> > giniPath;

    ALinearRegressor(Config* config, Dataset* dataset);
    ~ALinearRegressor();
    virtual void fit(int, float) = 0;
    int penalizeLasso(float learning_rate, float l1);
    void penalizeRidge(float learning_rate, float l2);
    void predict();
    double logLikelihood(const std::vector<int> &samples);
    void writeResults(std::string filename , std::vector<int> test);
    int getMinCoeff(std::set<int>& selected_features);
    double getCoeffNorm2(int feature);
    double getSpread(int feature);
    void eraseFeature(int i, int feature);
    void printSelectedFeatures();
    double getNorm2CoeffDiff(double* coeffs2);
    double rmse(const std::vector<int> &samples);
    double gini(const std::vector<int> &samples);
    void printResults(const std::vector<int> &train, const std::vector<int> &test);
    void writeGiniPath();

private:
    std::vector<size_t> reverse_sort_indexes(const std::vector<float> &v,
                                             const std::vector<int> &samples);

};

#endif  // ALINEARREGRESSOR_H_
