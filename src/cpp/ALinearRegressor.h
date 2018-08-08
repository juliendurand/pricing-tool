#ifndef ALINEARREGRESSOR_H_
#define ALINEARREGRESSOR_H_

#include <set>
#include <string>
#include <vector>

#include "Coefficients.h"
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
    std::unique_ptr<Coefficients> getCoeffs();
    void eraseAllFeatures();
    void eraseFeatures(const std::vector<int> &features);
    void addFeatures(const std::vector<int> &features);

protected:
    std::vector<double> coeffs;
    std::vector<double> weights;
    std::vector<double> stdev;
    std::vector<double> x0;
    std::vector<double> x1;
    std::vector<double> g;
};

#endif  // ALINEARREGRESSOR_H_
