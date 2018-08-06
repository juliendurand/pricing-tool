#include <iostream>

#include "config.h"
#include "SGDRegressor.h"


void fitToConvergence(SGDRegressor* model, long& i, int precision,
                      float stopCriterion){
    double minll = 1e30;
    int nbIterationsSinceMinimum = 0;
    int epoch = model->dataset->getSize() / model->getBlockSize();
    for(; nbIterationsSinceMinimum < precision; i++){
        model->fit();
        if(i % epoch == 0){
            std::cout << i * model->getBlockSize() << "th iteration : " << " minll " << minll << " iteration since min " << nbIterationsSinceMinimum << std::endl;
            model->printResults();
            model->predict(model->dataset->getTrain());
            double ll = model->logLikelihood(model->dataset->getTrain());
            if(ll < minll - stopCriterion) {
                minll = ll;
                nbIterationsSinceMinimum = 0;
            } else {
                nbIterationsSinceMinimum++;
            }
        }
    }
}

void backwardStepwise(SGDRegressor* model, long& i){
    std::cout << "Backward Stepwise" << std::endl;
    int epoch = model->dataset->getSize() / model->getBlockSize();
    for(;; i++){
        model->fit();
        if(i % (epoch) == 0){
            if(model->selected_features.size() > 0){
                int remove_feature = model->getMinCoeff(model->selected_features);
                model->storeFeatureInGiniPath(remove_feature);
                model->eraseFeatures({remove_feature});
            }
            else{
                model->storeFeatureInGiniPath(-1);
                break;
            }
        }
    }
}

void forwardStepwise(SGDRegressor* model, long& i, int maxNbFeatures){
    std::cout << "Forward Stepwise" << std::endl;
    model->eraseAllFeatures();
    int epoch = model->dataset->getSize() / model->getBlockSize();
    int k = 0;
    for(auto p : model->giniPath){
        int f = p.feature_idx;
        if(f >= 0){
            model->addFeatures({f});
            while(++i % epoch != 0){
                model->fit();
            }
            model->storeFeatureInGiniPath(f);
            if(++k >= maxNbFeatures){
                break;
            }
        }
    }
}

ALinearRegressor* fit(Config* config, Dataset* ds){
    int blocksize = 200;
    double learningRate = 0.0001;
    SGDRegressor* model = new SGDRegressor(config, ds, blocksize, learningRate);
    std::cout << std::endl << "Fit Model for " << config->nbFeaturesInModel
                           << " variables :" << std::endl;

    long i = 0;
    double stopCriterion = model->config->loss == "poisson" ? 0.00001 : 0.000001;
    fitToConvergence(model, i, 1, stopCriterion);
    model->printResults();
    backwardStepwise(model, i);

    int maxSortedFeatures = config->p;
    for(int k = 0; k < 6; k++){
        std::vector<int> bestFeatures = model->getBestFeatures(maxSortedFeatures, 0.0001);
        maxSortedFeatures = bestFeatures.size();
        forwardStepwise(model, i, maxSortedFeatures);
    }
    maxSortedFeatures = std::min(maxSortedFeatures, model->config->nbFeaturesInModel);
    std::vector<int> bestFeatures = model->getBestFeatures(maxSortedFeatures, 0.0002);
    forwardStepwise(model, i, maxSortedFeatures);
    model->eraseAllFeatures();
    model->addFeatures(bestFeatures);
    fitToConvergence(model, i,  5, stopCriterion);

    return model;
}

int main(int argc, char** argv){
    if(argc != 2){
        std::cout << "Invalid parameters. Expecting 1 parameter : [config file]." <<std::endl;
        return 1;
    }
    std::string config_filename = argv[1];

    Config config(config_filename, 0.2);
    Dataset ds(&config);
    ALinearRegressor* model = fit(&config, &ds);

    std::cout << std::endl
              << "Final results :" << std::endl
              << "---------------" << std::endl;

    model->printResults();
    model->printSelectedFeatures(model->selected_features.size());
    model->writeResults(ds.getSample());

    std::cout << std::endl << "Finished OK." << std::endl;
}
