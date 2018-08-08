#include "Coefficients.h"

#include <fstream>


Coefficients::Coefficients(Config* config,
    const std::vector<double>& coeffs,
    const std::vector<double>& weights,
    const std::set<int>& selected_features) :
    config(config),
    coeffs(coeffs),
    weights(weights),
    selected_features(selected_features)
{
}

std::unique_ptr<ModelResult> Coefficients::predict(Dataset* dataset,
        const std::vector<int> &samples)
{
    uint8_t* x = dataset->get_x();
    float* weight = dataset->get_weight();
    float* y = dataset->get_y();
    ModelResult* result = new ModelResult(samples.size(), config);
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

int Coefficients::getMinCoeff(){
    int minidx = -1;
    double minvalue = 0;
    for(int i : selected_features){
        double s = getCoeffGini(i);
        if(minidx == -1 || s < minvalue){
            minvalue = s;
            minidx = i;
        }
    }
    return minidx;
}

double Coefficients::getCoeffNorm2(int feature){
    if(feature < 0){
        return 0;
    }

    double sc = 0;
    double sw = 0;
    for(int j = config->offsets[feature]; j < config->offsets[feature + 1] ;
        j++){
        double c = std::exp(coeffs[j + 1]);
        double w = weights[j + 1];
        sc += c * c * w;
        sw += w;
    }
    return std::sqrt(sc / sw);
}

double Coefficients::getCoeffGini(int feature){
    if(feature < 0){
        return 0;
    }

    int nb_coeffs = config->offsets[feature + 1] - config->offsets[feature];
    std::vector<int> feature_idx(nb_coeffs);
    for(int i = 0; i < nb_coeffs; i++){
        feature_idx[i] = config->offsets[feature] + i;
    }
    std::sort(feature_idx.begin(), feature_idx.end(),
        [this](size_t i, size_t j) {
            return this->coeffs[i + 1] < this->coeffs[j + 1];
        }
    );

    double g = 0;
    double sc = 0;
    double sw = 0;
    for(int i : feature_idx){
        int j = i + 1;
        double w = weights[j];
        double c = std::exp(coeffs[j]) * w;
        g += w * (2 * sc + c);
        sc += c;
        sw += w;
    }
    g = 1 - g / (sc * sw);
    g = g < 0.0000001 ? 0 : g;
    return g * 100;
}

double Coefficients::getSpread100(int feature){
    if(feature < 0){
        return 0;
    }

    double minvalue = 100000000;
    double maxvalue = 0;

    for(int j = config->offsets[feature]; j < config->offsets[feature + 1] ;
        j++){
        float c = std::exp(coeffs[j + 1]);
        if(c < minvalue) minvalue = c;
        if(c > maxvalue) maxvalue = c;
    }
    return float(std::round((maxvalue / minvalue - 1) * 10000)) / 100;
}

double Coefficients::getSpread95(int feature){
    if(feature < 0){
        return 0;
    }

    int nb_coeffs = config->offsets[feature + 1] - config->offsets[feature];
    std::vector<int> feature_idx(nb_coeffs);
    for(int i = 0; i < nb_coeffs; i++){
        feature_idx[i] = config->offsets[feature] + i;
    }
    std::sort(feature_idx.begin(), feature_idx.end(),
        [this](size_t i, size_t j) {
            return this->coeffs[i + 1] < this->coeffs[j + 1];
        }
    );
    std::vector<double> cum_weight(nb_coeffs);
    for(int i = 0; i < nb_coeffs; i++){
        cum_weight[i] = (i > 0 ? cum_weight[i - 1] : 0) +
                        (weights[feature_idx[i] + 1] / weights[0]);
    }
    double minvalue = 0;
    double maxvalue = 0;
    for(int i = 0; i < nb_coeffs; i++){
        if(cum_weight[i] > 0.05){
            int j = feature_idx[i] + 1;
            minvalue = std::exp(coeffs[j]);
            break;
        }
    }
    for(int i = 0; i < nb_coeffs; i++){
        if(cum_weight[i] > 0.95){
            int j = feature_idx[i] + 1;
            maxvalue = std::exp(coeffs[j]);
            break;
        }
    }
    return float(std::round((maxvalue / minvalue - 1) * 10000)) / 100;
}

void Coefficients::writeResults(){
    std::ofstream coeffFile;
    coeffFile.open(config->resultPath + "coeffs.csv", std::ios::out);
    coeffFile << "Coeffs" << std::endl;
    for(double c : coeffs){
        coeffFile << c << std::endl;
    }
    coeffFile.close();
}
