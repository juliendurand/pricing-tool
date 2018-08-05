#include "ALinearRegressor.h"

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


ALinearRegressor::ALinearRegressor(Config* configuration, Dataset* ds)
{
    config = configuration;
    dataset = ds;
    n = config->n;
    p = config->p;
    nbCoeffs = config->m;
    offsets = config->offsets;
    features = config->features;
    x = dataset->get_x();
    exposure = dataset->get_weight();
    y = dataset->get_y();

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
        if(s > 0 && (weights[i] > std::sqrt(weights[0]) / 10)){
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
        if(featureIdx >= 0){
            std::cout << "Excluded feature " << feature << std::endl;
            eraseFeatures({featureIdx});
        } else {
            std::cout << "ERROR : Excluded feature " << feature
                      << " can not be found." << std::endl;
        }
    }

    ypred.reserve(n);
    dppred.reserve(n);
    giniPath = std::vector<FeatureResult>(selected_features.size() + 1,
                                          FeatureResult());

    g.reserve(nbCoeffs + 1);
    g2.reserve(nbCoeffs + 1);
    for(int j = 0; j < nbCoeffs + 1 ; j++){
        g[j] = 0;
        g2[j] = 0;
    }
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
    ss << std::setprecision(std::numeric_limits<int>::max());
    ss << d;
    return ss.str();
}

void ALinearRegressor::writeResults(std::vector<int> test){
    predict(test);
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
    // FIXME for intercept
    for(int j=0; j < nbCoeffs + 1; j++){
        double c = stdev[j] != 0 ? coeffs[j] / stdev[j] : 0;
        coeffFile << doubleToText(c) << std::endl;
    }
    coeffFile.close();

    std::ofstream selectedFeatureFile;
    selectedFeatureFile.open(config->resultPath + "features.csv", std::ios::out);
    selectedFeatureFile << "Feature,Gini,Spread 95/5,Spread 100/0" << std::endl;
    for(int i = 0; i < selected_features.size() + 1; i++){
        FeatureResult& fr = giniPath[i];
        int f = fr.feature_idx;
        selectedFeatureFile << fr.feature << ','
                            << fr.gini << ','
                            << getSpread95(f) << ','
                            << getSpread100(f)
                            << std::endl;
    }
    selectedFeatureFile.close();

    std::ofstream giniPathFile;
    giniPathFile.open(config->resultPath + "ginipath.csv", std::ios::out);
    giniPathFile << "Feature,Gini,CoeffGini,Norm,Spread 100/0,Spread 95/5,RMSE" << std::endl;
    for (FeatureResult& p : giniPath) {
        giniPathFile << p.feature << ","
                     << p.gini << ","
                     << p.coeffGini << ","
                     << p.norm << ","
                     << p.spread100 << ","
                     << p.spread95 << ","
                     << p.rmse
                     << std::endl;
    }
    giniPathFile.close();
}

int ALinearRegressor::getMinCoeff(std::set<int>& selected_features){
    int minidx = -1;
    double minvalue = 100000000;
    for(int i = 0; i< p; i++){
        double s = getCoeffGini(i); //getCoeffGini(i);//getCoeffNorm2(i); // getCoeffNorm2(i);// * getSpread95(i) / getSpread100(i);
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
        double c = stdev[j + 1] > 0 ? coeffs[j + 1] / stdev[j + 1] : 0;
        double w = weights[j + 1];
        sc += c * c * w;
        sw += w;
    }
    return std::sqrt(sc / sw);
}

double ALinearRegressor::getCoeffGini(int feature){
    if(feature < 0){
        return 0;
    }

    int nb_coeffs = offsets[feature + 1] - offsets[feature];
    std::vector<int> feature_idx(nb_coeffs);
    for(int i = 0; i < nb_coeffs; i++){
        feature_idx[i] = offsets[feature] + i;
    }
    std::sort(feature_idx.begin(), feature_idx.end(),
        [this](size_t i, size_t j) {
            return std::exp(this->coeffs[i + 1] / this->stdev[i + 1]) <
                   std::exp(this->coeffs[j + 1] / this->stdev[j + 1]);
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
        double c = stdev[j] != 0 ? std::exp(coeffs[j] /*/ stdev[j]*/) * w : 1;
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
    if(feature < 0){
        return 0;
    }

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
    if(feature < 0){
        return 0;
    }
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

void ALinearRegressor::storeFeatureInGiniPath(int f){
    int position = selected_features.size();
    FeatureResult fr;
    predict(dataset->test);
    if(position == 0){
        fr = {
            -1,
            "Intercept",
            gini(dataset->test),
            0,
            0,
            0,
            0,
            rmse(dataset->test),
            0
        };
        std::cout << "Storing Intercept." << std::endl;
    } else {
        fr = {
            f,
            config->features[f],
            gini(dataset->test),
            getCoeffGini(f),
            getCoeffNorm2(f),
            getSpread100(f),
            getSpread95(f),
            rmse(dataset->test),
            0
        };
        std::cout << "Storing[" << position << "] "
                  << fr.feature
                  << " Gini=" << fr.gini
                  << " Norm2=" << fr.norm
                  << " CGini=" << fr.coeffGini
                  << " Spread100=" << fr.spread100
                  << " Spread95=" << fr.spread95
                  << std::endl;
    }
    giniPath[position] = fr;
}

void ALinearRegressor::eraseAllFeatures(){
    std::vector<int> allFeatures(selected_features.begin(),
                                 selected_features.end());
    eraseFeatures(allFeatures);
}

void ALinearRegressor::eraseFeatures(const std::vector<int> &features){
    for(int f : features){
        selected_features.erase(f);
        for(int j = config->offsets[f];
            j < config->offsets[f + 1]; j++){
            coeffs[j + 1] = 0;
            x0[j + 1] = 0;
            x1[j + 1] = 0;
        }
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

void ALinearRegressor::printSelectedFeatures(int nbSelected){
    std::cout << "Selected Features :" <<std::endl;
    for(int i = 1; i < nbSelected + 1; i++){
        FeatureResult* p = &giniPath[i];
        std::cout << "        " << i << " : " << p->feature
                  << " [N2=" << p->norm
                  << ", CGini" << p->coeffGini * 100
                  << "%, Spread(100/0)=" << p->spread100 * 100
                  << "%, Spread(95/5)=" << p->spread95 * 100 << "%]"
                  << std::endl;
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
    predict(dataset->train);
    predict(dataset->test);
    predict(dataset->sample);
    std::cout << "gini(train=" << gini(dataset->train)
              << ", test="     << gini(dataset->test)
              << ", sample="     << gini(dataset->sample) << ")"
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

void ALinearRegressor::sortFeatures(){
    std::vector<int> diff(p+1);
    for(int i = 1; i < giniPath.size(); i++){
        giniPath[i].diffGini = giniPath[i].gini - giniPath[i - 1].gini;
    }
    std::sort(giniPath.begin() + 1, giniPath.end(),
        [diff](FeatureResult& i, FeatureResult& j) {
            return i.diffGini > j.diffGini;
        }
    );
}

void ALinearRegressor::sortFeatures(int maxNbFeatures){
    std::vector<int> diff(p+1);
    for(int i = 1; i < giniPath.size(); i++){
        giniPath[i].diffGini = giniPath[i].gini - giniPath[i - 1].gini;
    }
    std::sort(giniPath.begin() + 1, std::min(giniPath.end(), giniPath.begin() + maxNbFeatures+ 1),
        [diff](FeatureResult& i, FeatureResult& j) {
            return i.diffGini > j.diffGini;
        }
    );
}

std::vector<int> ALinearRegressor::getBestFeatures(int maxNbFeatures,
                                                   double treshold){
    sortFeatures(maxNbFeatures);
    std::vector<int> bestFeatures;
    for (auto p = giniPath.begin() + 1; p != giniPath.end(); p++) {
        if(p->diffGini > treshold){
            bestFeatures.push_back(p->feature_idx);
        } else {
            break;
        }
        if(bestFeatures.size() >= maxNbFeatures){
            std::cout << "Stop adding features : max nb features reached("
                      << maxNbFeatures << (").") << std::endl;
            break;
        }
    }
    return bestFeatures;
}
