#include "Coefficients.h"

#include <fstream>


Coefficients::Coefficients(Config* config,
    const std::vector<double>& coeffs, const std::set<int>& selected_features) :
    config(config),
    coeffs(coeffs),
    selected_features(selected_features)
{
}

std::unique_ptr<ModelResult> Coefficients::predict(Dataset* dataset,
        const std::vector<int> &samples)
{
    uint8_t* x = dataset->get_x();
    float* weight = dataset->get_weight();
    float* y = dataset->get_y();
    ModelResult* result = new ModelResult(samples.size(), config->loss);
    int p = config->p;
    int j = 0;
    for(int i : samples){
        double dp = coeffs[0];
        for(int j : selected_features){
            int k = config->offsets[j] + x[p * i + j] + 1;
            dp += coeffs[k];
        }
        result->setObservation(j, i, y[i], exp(dp) * weight[i], weight[i],
                               dp);
        ++j;
    }
    return std::unique_ptr<ModelResult>(result);
}

void Coefficients::writeResults(std::string resultPath){
    std::ofstream coeffFile;
    coeffFile.open(resultPath + "coeffs.csv", std::ios::out);
    coeffFile << "Coeffs" << std::endl;
    for(double c : coeffs){
        coeffFile << c << std::endl;
    }
    coeffFile.close();
}
