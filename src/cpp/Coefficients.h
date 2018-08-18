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
        const std::vector<float>& coeffs,
        const std::vector<float>& weights,
        const std::set<int>& selected_features);
    std::unique_ptr<ModelResult> predict(Dataset* dataset,
        const std::vector<int> &samples);
    int getMinCoeff();
    float getCoeffNorm2(int feature);
    float getCoeffGini(int feature);
    float getSpread95(int feature);
    float getSpread100(int feature);
    void saveResults();

private:
    Config* config;
    std::vector<float> coeffs;
    std::vector<float> weights;
    std::set<int> selected_features;
};

#endif  // COEFFICIENTS_H_
