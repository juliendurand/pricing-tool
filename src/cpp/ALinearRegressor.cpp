#include <algorithm>
#include <assert.h>
#include <cinttypes>
#include <cmath>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "array.h"
#include "ALinearRegressor.h"


ALinearRegressor::ALinearRegressor(Config* configuration, Dataset* ds)
{
    config = configuration;
    dataset = ds;
    n = config->n;
    p = config->p;
    nbCoeffs = config->m;
    offsets = config->offsets;
    features = config->features;
    x = dataset->x_data->getData();
    exposure = dataset->weight_data->getData();
    y = dataset->y_data->getData();

    coeffs = new double[nbCoeffs + 1];
    weights = new double[nbCoeffs + 1];
    stdev = new double[nbCoeffs + 1];
    x0 = new double[nbCoeffs + 1];
    x1 = new double[nbCoeffs + 1];


    for(int i = 0; i < nbCoeffs + 1; i++){
        coeffs[i] = 0;
        weights[i] = 0;
    }

    for(int i : dataset->train){
        weights[0] += exposure[i];
        uint8_t*  xi = x + p * i;
        for(int j = 0; j < p; j++){
            weights[xi[j] + offsets[j] + 1] += exposure[i];
        }
    }

    stdev[0] = 1;
    x0[0] = 0;
    x1[0] = 1;
    for(int i = 1; i < nbCoeffs + 1; i++){
        double w = weights[i] / weights[0];
        double s= std::sqrt(w - w * w);
        stdev[i] = s;
        if(s > 0){
            x1[i] = (1 - w) / s;
            x0[i] = (0 - w) / s;
        } else {
            x1[i] = 0;
            x0[i] = 0;
        }
    }

    for(int i=0; i < p; i++){
        selected_features.insert(i);
    }
    for(std::string feature : config->excludedFeatures){
        int featureIdx = config->getFeatureIndex(feature);
        eraseFeature(0, featureIdx);
    }

    ypred.reserve(n);
    dppred.reserve(n);
    giniPath.reserve(p + 1);
}

ALinearRegressor::~ALinearRegressor()
{
    delete[] coeffs;
    delete[] weights;
    delete[] stdev;
    delete[] x0;
    delete[] x1;
}

int ALinearRegressor::penalizeLasso(float learning_rate, float l1){
    int nb_coeffs_non_zero = 0;
    for(int j = 0; j < nbCoeffs + 1; j++){
        float c = coeffs[j];
        if(c > l1 * learning_rate){
            coeffs[j] -= l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else if(c < -l1 * learning_rate){
            coeffs[j] += l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else {
            coeffs[j] = 0;
        }
    }
    return nb_coeffs_non_zero;
}

void ALinearRegressor::penalizeRidge(float learning_rate, float l2){
    for(int j = 1; j < nbCoeffs + 1; j++){
        coeffs[j] *= (1 - l2 * learning_rate);
    }
}

void ALinearRegressor::predict(const std::vector<int> &samples){
    double dp0 = 0;
    for(int j = 1; j < nbCoeffs; j++){
        dp0 += x0[j] * coeffs[j];
    }
    for(int i : samples){
        std::cout << i << std::endl;
        uint8_t* xi = x + p * i;
        double dp = coeffs[0] + dp0;
        for(int j : selected_features){
            int k = offsets[j]+ xi[j] + 1;
            dp += (x1[k] - x0[k]) * coeffs[k];
        }
        dppred[i] = dp;
        ypred[i] = exp(dp) * exposure[i];
    }
}

double ALinearRegressor::logLikelihood(const std::vector<int> &samples){
    double ll = 0;
    if(config->loss == "gaussian"){
        for(int i : samples){
            double dp = dppred[i];
            double ei = exposure[i];
            ll += std::sqrt(ei * dp - y[i]);
        }
    } else if(config->loss == "poisson") {
        for(int i : samples){
            double dp = dppred[i];
            double ei = exposure[i];
            ll += ei * std::exp(dp) - y[i] * dp + std::log(ei);
        }
    } else if(config->loss == "gamma") {
        for(int i : samples){
            double dp = dppred[i];
            double ei = exposure[i];
            ll += y[i] / (ei * std::exp(dp)) + dp;
        }
    } else {
        throw std::invalid_argument( "Received invalid loss function." );
    }
    return ll / samples.size();
}

std::string doubleToText(const double & d)
{
    std::stringstream ss;
    ss << std::setprecision( std::numeric_limits<int>::max() );
    ss << d;
    return ss.str();
}

void ALinearRegressor::writeResults(std::vector<int> test){
    std::cout << "Writting results." << std::endl;

    std::ofstream resultFile;
    resultFile.open(config->resultPath + "results.csv", std::ios::out);
    resultFile << "row,exposure,target,prediction" << std::endl;
    for(int i : test){
        resultFile << i << "," << exposure[i] << "," << y[i] << ","
            << ypred[i] << std::endl;
    }
    resultFile.close();

    std::ofstream coeffFile;
    coeffFile.open(config->resultPath + "coeffs.csv", std::ios::out);
    coeffFile << "Coeffs" << std::endl;
    for(int j=0; j < nbCoeffs + 1; j++){
        double c = stdev[j] != 0 ? coeffs[j] / stdev[j] : 0;
        coeffFile << doubleToText(c) << std::endl;
    }
    coeffFile.close();
}

int ALinearRegressor::getMinCoeff(std::set<int>& selected_features){
    int minidx = -1;
    double minvalue = 100000000;
    for(int i = 0; i< p; i++){
        double s = getCoeffNorm2(i); //getCoeffGini(i); // getCoeffNorm2(i);// * getSpread95(i) / getSpread100(i);
        //std::cout << config->features[i] << " " << getCoeffGini(i) << std::endl;
        if((selected_features.count(i) > 0) && (s < minvalue)){
            minvalue = s;
            minidx = i;
        }
    }
    return minidx;
}

double ALinearRegressor::getCoeffNorm2(int feature){
    double sc = 0;
    double sw = 0;
    for(int j = offsets[feature]; j < offsets[feature + 1] ; j++){
        double c = coeffs[j + 1];
        double w = weights[j + 1];
        sc += c * c * w;
        sw += w;
    }
    return std::sqrt(sc / sw);
}

double ALinearRegressor::getCoeffGini(int feature){
    int nb_coeffs = offsets[feature + 1] - offsets[feature];
    std::vector<int> feature_idx(nb_coeffs);
    for(int i = 0; i < nb_coeffs; i++){
        feature_idx[i] = offsets[feature] + i;
    }
    std::sort(feature_idx.begin(), feature_idx.end(),
        [this](size_t i, size_t j) {
            return std::exp(this->coeffs[i + 1] / this->stdev[i + 1]) < std::exp(this->coeffs[j + 1] / this->stdev[j + 1]);
        }
    );

    //std::cout << std::endl << "sorted" << std::endl;
    //for(auto i : feature_idx){
    //    std::cout << i << " " << std::exp(coeffs[i + 1] / stdev[i + 1])<< " " << weights[i + 1] << std::endl;
    //}

    //std::cout << "caculate" << std::endl;
    double g = 0;
    double sc = 0;
    double sw = 0;
    for(int i : feature_idx){
        int j = i + 1;
        double w = weights[j];
        double c = stdev[j] != 0 ? std::exp(coeffs[j] / stdev[j]) * w : 1;
        g += w * (2 * sc + c);
        sc += c;
        sw += w;
        //std::cout << j << " " << g << " " << c << " " << w << " " << sc << " " << sw << std::endl;
    }
    g = 1 - g / (sc * sw);
    //std::cout << "gini " << config->features[feature] << " " << g << std::endl;
    return g < 0.0000001 ? 0 : g;
}

double ALinearRegressor::getSpread100(int feature){
    double minvalue = 100000000;
    double maxvalue = 0;

    for(int j = offsets[feature]; j < offsets[feature + 1] ; j++){
        float c = std::exp(coeffs[j + 1]  / stdev[j + 1]);
        if(c < minvalue) minvalue = c;
        if(c > maxvalue) maxvalue = c;
    }

    return float(std::round((maxvalue / minvalue - 1) * 10000)) / 100;
}

double ALinearRegressor::getSpread95(int feature){
    int nb_coeffs = offsets[feature + 1] - offsets[feature];
    std::vector<int> feature_idx(nb_coeffs);
    for(int i = 0; i < nb_coeffs; i++){
        feature_idx[i] = offsets[feature] + i;
    }
    std::sort(feature_idx.begin(), feature_idx.end(),
        [this](size_t i, size_t j) {
            return std::exp(this->coeffs[i + 1] / this->stdev[i + 1]) < std::exp(this->coeffs[j + 1] / this->stdev[j + 1]);
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
            if(stdev[j] == 0){
                continue;
            }
            minvalue = std::exp(coeffs[j]/ stdev[j]);
            break;
        }
    }
    for(int i = 0; i < nb_coeffs; i++){
        if(cum_weight[i] > 0.95){
            int j = feature_idx[i] + 1;
            if(stdev[j] == 0){
                continue;
            }
            maxvalue = std::exp(coeffs[j]/ stdev[j]);
            break;
        }
    }

    return float(std::round((maxvalue / minvalue - 1) * 10000)) / 100;
}

void ALinearRegressor::eraseFeature(int i, int remove_feature){
    predict(dataset->test);
    if(remove_feature < 0){
        FeatureResult fr = {
            -1,
            "Intercept",
            gini(dataset->test),
            0,
            0,
            0,
            0,
            rmse(dataset->test)
        };
        giniPath.push_back(fr);
        return;
    }

    std::string feature = config->features[remove_feature];
    std::cout << i << " : Removing[" << selected_features.size()
              << "] " << feature
              << " Norm2=" << getCoeffNorm2(remove_feature)
              << " CGini=" << getCoeffGini(remove_feature)
              << std::endl;

    FeatureResult fr = {
        remove_feature,
        features[remove_feature],
        gini(dataset->test),
        getCoeffGini(remove_feature),
        getCoeffNorm2(remove_feature),
        getSpread100(remove_feature),
        getSpread95(remove_feature),
        rmse(dataset->test)
    };
    giniPath.push_back(fr);

    selected_features.erase(remove_feature);
    for(int j = config->offsets[remove_feature];
        j < config->offsets[remove_feature + 1]; j++){
        coeffs[j + 1] = 0;
        x0[j + 1] = 0;
        x1[j + 1] = 0;
    }
}

void ALinearRegressor::addFeatures(const std::vector<int> &features){
    for(int f : features){
        selected_features.insert(f);
        for(int j = config->offsets[f];
            j < config->offsets[f + 1]; j++){
            int i = j + 1;
            double w = weights[i] / weights[0];
            double s= std::sqrt(w - w * w);
            stdev[i] = s;
            if(s > 0){
                x1[i] = (1 - w) / s;
                x0[i] = (0 - w) / s;
            } else {
                x1[i] = 0;
                x0[i] = 0;
            }
        }
    }
}

void ALinearRegressor::printSelectedFeatures(){
    std::map<double, int> keep_features;
    for(auto f : selected_features){
        double v = getSpread100(f);
        // FIXME : this does not work if two features have the same spread !
        keep_features[v] = f;
    }

    int i = 1;
    std::cout << "Selected Features :" <<std::endl;
    for(auto kv = keep_features.rbegin(); kv != keep_features.rend(); kv++){
        int featureIdx = kv->second ;
        std::cout << "        " << i << " : " << features[featureIdx] << " [N2="
                  << getCoeffNorm2(featureIdx) << ", Spread(100/0)="
                  << getSpread100(featureIdx) << "%]" << std::endl;
        i++;
    }
}

double ALinearRegressor::getNorm2CoeffDiff(double* coeffs2){
    int size = 0;
    double s = 0;
    for(int i = 0; i < config->m + 1; i++){
        if(coeffs[i] != 0 || coeffs2[i] != 0){
            size++;
            double diff = (coeffs[i] - coeffs2[i]) / stdev[i];
            s += diff * diff ;
        }
    }
    return std::sqrt(s /  size);
}

double ALinearRegressor::rmse(const std::vector<int> &samples){
    double rmse = 0;
    double sexp = 0;
    for(int j=0; j < samples.size(); j++){
        int i = samples[j];
        double e = y[i] - ypred[i];
        rmse += e * e;
        sexp += exposure[i];
    }
    return std::sqrt(rmse/sexp);
}

double ALinearRegressor::gini(const std::vector<int> &samples){
    std::vector<size_t> idx = reverse_sort_indexes(ypred, exposure, samples);
    double exposure_sum = 0;
    double obs_sum = 0;
    double rank_obs_sum = 0;
    for(int i : idx){
        int obs = samples[i];
        double e = exposure[obs];
        exposure_sum += e;
        obs_sum += y[obs];
        rank_obs_sum += y[obs] * (exposure_sum - 0.5 * e);
    }
    return 1 - (2 / (exposure_sum * obs_sum)) * rank_obs_sum;
}

void ALinearRegressor::printResults(){
    std::cout << "a" << std::endl;
    predict(dataset->train);
    std::cout << "b" << std::endl;
    predict(dataset->test);
    std::cout << "c" << std::endl;
    std::cout << "gini(train=" << gini(dataset->train)
              << ", test="     << gini(dataset->test) << ")"
              << " | "
              << "ll(train=" << logLikelihood(dataset->train)
              << ", test="   << logLikelihood(dataset->test) << ")"
              << std::endl;
}

std::vector<size_t> ALinearRegressor::reverse_sort_indexes(
        const std::vector<float> &v, float* w, const std::vector<int> &samples)
{
    // initialize original index locations
    std::vector<size_t> idx(samples.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
        [&v, w, &samples](size_t j1, size_t j2) {
            int i1 = samples[j1];
            int i2 = samples[j2];
            return v[i1] / w[i1] > v[i2] / w[i2];
        }
    );

  return idx;
}

void ALinearRegressor::writeGiniPath(){
    std::cout << "Writting Gini Path." << std::endl << std::endl;
    std::ofstream giniPathFile;
    giniPathFile.open(config->resultPath + "ginipath.csv", std::ios::out);
    giniPathFile << "Feature,Gini,CoeffGini,Norm,Spread 100/0,Spread 95/5,RMSE" << std::endl;
    for (auto p = giniPath.rbegin(); p != giniPath.rend(); p++) {
        giniPathFile << p->feature << "," << p->gini << "," << p->coeffGini
                     << "," << p->norm << "," << p->spread100 << ","
                     << p->spread95 << "," << p->rmse
                     << std::endl;
    }
    giniPathFile.close();
}

std::vector<int> ALinearRegressor::getBestFeatures(){
    std::cout << "Best Features : " << std::endl;
    std::vector<int> bestFeatures;
    double previousGini = 0;
    for (auto p = giniPath.rbegin(); p != giniPath.rend(); p++) {
        int f = p->feature_idx;
        if(f >= 0){
            if(p->gini - previousGini > 0.001){
                std::cout << config->features[f] << " : gini=" << p->gini << std::endl;
                bestFeatures.push_back(f);
            }
            if(p->gini > previousGini){
                previousGini = p->gini;
            }
        }
    }
    return bestFeatures;
}
