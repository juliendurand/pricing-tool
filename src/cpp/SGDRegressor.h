#ifndef SGDREGRESSOR_H_
#define SGDREGRESSOR_H_

#include <set>
#include <string>
#include <vector>

#include "Coefficients.h"
#include "Config.h"
#include "Dataset.h"


class SGDRegressor
{
public:
    Config* config;
    Dataset* dataset;

    SGDRegressor(Config* config, Dataset* dataset);
    virtual ~SGDRegressor();
    virtual void fit();
    virtual std::unique_ptr<Coefficients> getCoeffs();
    std::set<int> getSelectedFeatures() const;
    void fitEpoch(long& i, float nb_epoch);
    void fitUntilConvergence(long& i, int precision, float stopCriterion);
    void eraseAllFeatures();
    void eraseFeatures(const std::vector<int> &features);
    void addFeatures(const std::vector<int> &features);
    void printResults();

private:
    int blocksize;
    float learningRate;
    std::set<int> selected_features;
    std::vector<double> coeffs;
    std::vector<double> update;
    std::vector<double> weights;
    std::vector<double> stdev;
    std::vector<double> x0;
    std::vector<double> x1;
    std::vector<double> g;

    double (*gradLoss)(double, double, double);
    void selectGradLoss(const std::string loss);
    void fitIntercept();
};

#endif  // SGDREGRESSOR_H_
