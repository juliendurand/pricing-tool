#ifndef COEFFICIENTS_H_
#define COEFFICIENTS_H_

#include <string>
#include <set>
#include <vector>

#include "config.h"
#include "Dataset.h"
#include "ModelResult.h"


class Coefficients
{
public:
    Coefficients(Config* config,
        const std::vector<double>& coeffs,
        const std::vector<double>& weights,
        const std::set<int>& selected_features);
    std::unique_ptr<ModelResult> predict(Dataset* dataset,
        const std::vector<int> &samples);
    int getMinCoeff();
    double getCoeffNorm2(int feature);
    double getCoeffGini(int feature);
    double getSpread95(int feature);
    double getSpread100(int feature);
    void writeResults();

private:
    Config* config;
    std::vector<double> coeffs;
    std::vector<double> weights;
    std::set<int> selected_features;
};

#endif  // COEFFICIENTS_H_
