#include "SGDRegressor.h"

#include <algorithm>
#include <iostream>


SGDRegressor::SGDRegressor(Config* config, Dataset* dataset):
    config(config),
    dataset(dataset),
    blocksize(200),
    learningRate(0.0001),
    momentum(0.90),
    coeffs(config->m + 1, 0),
    weights(config->m + 1, 0),
    stdev(config->m + 1, 0),
    x0(config->m + 1, 0),
    x1(config->m + 1, 0),
    g(config->m + 1, 0)
{
    int p = config->p;
    uint8_t* x = dataset->get_x();
    float* weight = dataset->get_weight();

    selectGradLoss(config->loss);

    // Calculates active exposure for every modality
    for(int i : dataset->getTrain()){
        weights[0] += weight[i];
        for(int j = 0; j < p; j++){
            weights[x[p * i + j] + config->offsets[j]] += weight[i];
        }
    }

    // Normalization of data (scaling)
    for(int i = 0; i < config->m + 1; i++){
        float w = weights[i] / weights[0];
        float s = std::sqrt(w - w * w);
        stdev[i] = s;
        if(i == 0){
            // intercept is always equals to 1
            x0[0] = 1;
            x1[0] = 0;
        }else if(s > 0 && (weights[i] > std::min(20.0f,
                                        std::sqrt(weights[0]) / 10))){
            x0[i] = (0 - w) / s;
            x1[i] = (1 - w) / s - x0[i];
        } else {
            x0[i] = 0;
            x1[i] = 0;
        }
    }

    // add all features and then excludes as required from configuration
    for(int i = 0; i < p; i++){
            addFeatures({i});
    }
    for(std::string feature : config->excludedFeatures){
        int featureIdx = config->getFeatureIndex(feature);
        if(featureIdx >= 0){
            std::cout << "Exclude feature " << feature << std::endl;
            eraseFeatures({featureIdx});
        } else {
            std::cout << "WARNING : Excluded feature " << feature
                      << " can not be found." << std::endl;
        }
    }

    // Ensures warm start
    fitIntercept();
}

void SGDRegressor::selectGradLoss(const std::string loss){
    if(loss == "gaussian"){
        gradLoss = [](float y, float dp, float weight){
            return y - dp * weight;
        };
        std::cout << "Using gaussian loss" << std::endl;
    } else if(loss == "poisson") {
        gradLoss = [](float y, float dp, float weight){
            return y - std::exp(dp) * weight;
        };
        std::cout << "Using poisson loss" << std::endl;
    } else if(loss == "gamma") {
        gradLoss = [](float y, float dp, float weight){
            return y / (std::exp(dp) * weight) - 1;
        };
        std::cout << "Using gamma loss" << std::endl;
    } else {
        throw std::invalid_argument("Received invalid loss function.");
    }
}


// Provides a good estimates for the intercept in order to get a "Warm Start".
void SGDRegressor::fitIntercept()
{
    // This calculation is correct only if :
    //   - all coefficients are set to 0 ;
    //   - or the features have *all* been normalized.
    float* weight = dataset->get_weight();
    float* y = dataset->get_y();

    float s = 0;
    float w = 0;
    for(int i : dataset->getTrain()){
        s += y[i];
        w += weight[i];
    }

    if(config->loss == "gaussian"){
        coeffs[0] = s / w;
    } else if(config->loss == "poisson") {
        coeffs[0] = std::log(s / w);
    } else if(config->loss == "gamma") {
        coeffs[0] = std::log(s / w);
    } else {
        throw std::invalid_argument( "Received invalid loss function." );
    }
}

// Stochastic Gradient Descent with accelerated momentum and mini-batch
void SGDRegressor::fit()
{
    int p = config->p;
    int m = selected_modality_list.size();
    int f = selected_features_list.size();
    uint8_t* x = dataset->get_x();
    float* weight = dataset->get_weight();
    float* y = dataset->get_y();
    std::vector<float> update(config->m + 1, 0); // delta for next update
    // also using : x0, x1, coeffs, blocksize, config->offsets, g,
    //              momentum, learningRate, gradLoss

    // dot-product value for intercept + null observations
    float dp0 = 0;
    for(int l = 0; l < m; l++){
        int j = selected_modality_list[l];
        dp0 += x0[j] * coeffs[j];
    }

    for(int b = 0; b < blocksize; b++){ // mini-batch
        int i = dataset->next(); // get a random observation
        int row = p * i;

        float dp = dp0;
        for(int l = 0; l < f; l++){
            int j = selected_features_list[l];
            int k = config->offsets[j] + x[row + j];
            dp += x1[k] * coeffs[k];
        }

        // calculates the error with the appropriate loss function
        float r = gradLoss(y[i], dp, weight[i]);

        // calculate the base update for each modality
        for(int l = 0; l < f; l++){
            int j = selected_features_list[l];
            int k = config->offsets[j] + x[row + j];
            update[k] += r;
        }

        // total error for the mini-batch
        update[0] += r;
    }

    // update each modality with momentum
    for(int l = 0; l < m; l++){
        int j = selected_modality_list[l];
        float grad = (update[j] * x1[j] + update[0] * x0[j]) / blocksize;
        g[j] = momentum * g[j] + grad;
        coeffs[j] += learningRate * g[j];
    }
}

// Return un-normalized coefficients (final regression coefficients)
std::unique_ptr<Coefficients> SGDRegressor::getCoeffs()
{
    std::vector<float> results(coeffs.size(), 0);
    results[0] = coeffs[0];
    for(int i : selected_features){
        for(int j = config->offsets[i]; j < config->offsets[i + 1]; j++){
            if(stdev[j] != 0){
                results[j] = coeffs[j] / stdev[j];
                results[0] -= results[j] * (weights[j] / weights[0]);
            }
        }
    }

    auto ptr = new Coefficients(config, results, weights, selected_features);
    return std::unique_ptr<Coefficients>(ptr);
}

std::set<int> SGDRegressor::getSelectedFeatures() const
{
    return selected_features;
}

// Fit as many blocks as required to reach the size of the training set. Note
// that due to the randomness of the picking process, there is no guarantee
// that all observations will be used. (some others may be used several times).
void SGDRegressor::fitEpoch(long& i, float nb_epoch)
{
    int epoch = dataset->getSize() / blocksize;
    int nb_blocks = nb_epoch * epoch;
    for(int j=0; j < nb_blocks; ++j){
        fit();
        ++i;
    }
}

void SGDRegressor::fitUntilConvergence(long& i, int precision,
                      float stopCriterion)
{
    float minll = 1e30;
    int nbIterationsSinceMinimum = 0;
    int epoch = dataset->getSize() / blocksize;
    while(nbIterationsSinceMinimum < precision){
        fit();
        if(i % epoch == 0){
            std::cout << i * blocksize << "th iteration : "
                      << " minll " << minll
                      << " iteration since min " << nbIterationsSinceMinimum
                      << std::endl;
            //printResults();
            auto coeffs = getCoeffs();
            auto trainResult = coeffs->predict(dataset, dataset->getTrain());
            float ll = trainResult->logLikelihood();
            if(ll < minll - stopCriterion) {
                minll = ll;
                nbIterationsSinceMinimum = 0;
            } else {
                nbIterationsSinceMinimum++;
            }
        }
        ++i;
    }
}

void SGDRegressor::eraseAllFeatures()
{
    std::vector<int> allFeatures(selected_features.begin(),
                                 selected_features.end());
    eraseFeatures(allFeatures);
}

void SGDRegressor::eraseFeatures(const std::vector<int> &features)
{
    for(int f : features){
        selected_features.erase(f);
    }
    initFeatures(features);
}

void SGDRegressor::addFeatures(const std::vector<int> &features)
{
    for(int f : features){
        selected_features.insert(f);
    }
    initFeatures(features);
}

void SGDRegressor::initFeatures(const std::vector<int> &features)
{
    for(int f : features){
        for(int j = config->offsets[f]; j < config->offsets[f + 1]; j++){
            coeffs[j] = 0; // reset regression coefficient to 0.
        }
    }

    selected_modality_list.clear();
    selected_modality_list.push_back(0);
    for(int i : selected_features){
        for(int j = config->offsets[i]; j < config->offsets[i + 1]; j++){
            selected_modality_list.push_back(j);
        }
    }

    selected_features_list.clear();
    for(int i : selected_features){
        selected_features_list.push_back(i);
    }
}

void SGDRegressor::printResults()
{
    auto c = getCoeffs();
    std::unique_ptr<ModelResult> trainResult = c->predict(dataset,
        dataset->getTrain());
    std::unique_ptr<ModelResult> testResult = c->predict(dataset,
        dataset->getTest());
    std::unique_ptr<ModelResult> sampleResult = c->predict(dataset,
        dataset->getSample());
    std::cout << "gini(train=" << trainResult->gini()
              << ", test="     << testResult->gini()
              << ", sample="     << sampleResult->gini() << ")"
              << " | "
              << "ll(train=" << trainResult->logLikelihood()
              << ", test="   << testResult->logLikelihood() << ")"
              << std::endl;
}
