#ifndef SGDREGRESSOR_H_
#define SGDREGRESSOR_H_

#include <set>
#include <string>
#include <vector>

#include "Coefficients.h"
#include "Config.h"
#include "Dataset.h"


// Algorithm fitting actuarial glm regression model for P&C technical pricing.
// The algorithm is based on a stochatic gradient descent with accelerated
// momentum and can fit both poisson and gamma loss function. The list of
// feature to include or exclude from the fit can by dynamically modified to
// enable fast feature selection. Several fit functions are provided to
// accomodate several needs (fit, fitEpoch, fitUntilConvergence).
//
// Params:
//      - config : configuration for the glm regression ;
//      - dataset : the dataset to use for the fit.
//
// Usage : set the list of active feature and use the fit* functions to
//         calculate the regression coefficients for the dataset.

class SGDRegressor
{
public:
    Config* config;
    Dataset* dataset;

    SGDRegressor(Config* config, Dataset* dataset);
    void fit();
    std::unique_ptr<Coefficients> getCoeffs();
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
    std::vector<double> coeffs; // regression coefficients.
    std::vector<double> update; // delta for next update (temporary data).
    std::vector<double> weights; // sum of active exposure for each modality.
    std::vector<double> stdev; // standard deviation for each modality.
    std::vector<double> x0; // normalized 0 value for each modality.
    std::vector<double> x1; // normalized 0 value for each modality.
    std::vector<double> g; // momentum

    double (*gradLoss)(double, double, double);
    void selectGradLoss(const std::string loss);
    void fitIntercept(); // fit intercept for warm start
};

#endif  // SGDREGRESSOR_H_
