#include "ModelResult.h"

#include <cmath>
#include <fstream>
#include <numeric>


ModelResult::ModelResult(const int size, Config* config) :
    config(config)
{
    id.resize(size);
    y.resize(size);
    y_pred.resize(size);
    weights.resize(size);
    dp.resize(size);
}

void ModelResult::setObservation(int position, int id, double y, double y_pred,
        double weight, double dp)
{
    this->id[position] = id;
    this->y[position] = y;
    this->y_pred[position] = y_pred;
    this->weights[position] = weight;
    this->dp[position] = dp;
}

// Calculates the log-likelihood for several glm loss functions
double ModelResult::logLikelihood()
{
    double ll = 0;
    std::string loss = config->loss;
    if(loss == "gaussian"){
        for(int i = 0; i < y.size(); i++){
            double e = (y_pred[i] - y[i]);
            ll += e * e / 2;
        }
    } else if(loss == "poisson") {
        for(int i = 0; i < y.size(); i++){
            ll += y_pred[i] - y[i] * dp[i] + std::log(weights[i]);
        }
    } else if(loss == "gamma") {
        for(int i = 0; i < y.size(); i++){
            ll += y[i] / y_pred[i] + dp[i];
        }
    } else {
        throw std::invalid_argument( "Received invalid loss function." );
    }
    return ll / y.size();
}

// Calculates the oot mean square error
double ModelResult::rmse()
{
    double rmse = 0;
    double sexp = 0;
    for(int i = 0; i < y.size(); i++){
        double e = y[i] - y_pred[i];
        rmse += e * e;
        sexp += weights[i];
    }
    return std::sqrt(rmse/sexp);
}

// Calculate the gini coefficient of the prediction.
double ModelResult::gini()
{
    std::vector<size_t> idx = reverse_sort_indexes(y_pred, weights);
    double exposure_sum = 0;
    double obs_sum = 0;
    double rank_obs_sum = 0;
    for(int i : idx){
        exposure_sum += weights[i];
        obs_sum += y[i];
        rank_obs_sum += y[i] * (exposure_sum - 0.5 * weights[i]);
    }
    return 1 - (2 / (exposure_sum * obs_sum)) * rank_obs_sum;
}

const std::vector<size_t> ModelResult::reverse_sort_indexes(
        const std::vector<double>& v, const std::vector<double>& w)
{
    // initialize original index locations
    std::vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
        [&v, w](size_t i1, size_t i2) {
            return v[i1] / w[i1] > v[i2] / w[i2];
        }
    );

  return idx;
}

// Persists the prediction in the filesystem
void ModelResult::saveResults()
{
    std::cout << std::endl << "Saving results." << std::endl;

    std::ofstream resultFile;
    resultFile.open(config->resultPath + "results.csv", std::ios::out);
    resultFile << "row,exposure,target,prediction" << std::endl;
    for(int i = 0; i < y.size(); ++i){
        resultFile << i << "," << weights[i] << "," << y[i] << ","
            << y_pred[i] << std::endl;
    }
    resultFile.close();
}
