#ifndef COEFFICIENTS_H_
#define COEFFICIENTS_H_

#include <string>
#include <set>

#include "config.h"
#include "Dataset.h"
#include "ModelResult.h"


class Coefficients
{
public:
    Coefficients(Config* config,
        const std::vector<double>& coeffs,
        const std::set<int>& selected_features);
    std::unique_ptr<ModelResult> predict(Dataset* dataset,
        const std::vector<int> &samples);
    void writeResults(std::string resultPath);

private:
    Config* config;
    std::vector<double> coeffs;
    std::set<int> selected_features;
};

#endif  // COEFFICIENTS_H_
