#include <iostream>

#include "SGDRegressor.h"


void fitEpoch(SGDRegressor* model, long& i, float nb_epoch){
    int epoch = model->dataset->getSize() / model->getBlockSize();
    int nb_blocks = nb_epoch * epoch;
    for(int j=0; j < nb_blocks; ++j){
        model->fit();
        ++i;
    }
}

void fitUntilConvergence(SGDRegressor* model, long& i, int precision,
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
    for(;;){
        fitEpoch(model, i, 1);
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

void forwardStepwise(SGDRegressor* model, long& i, int maxNbFeatures){
    std::cout << "Forward Stepwise" << std::endl;
    model->eraseAllFeatures();
    for(auto p : model->giniPath){
        int f = p.feature_idx;
        if(f >= 0){
            model->addFeatures({f});
            fitEpoch(model, i, 1);
            model->storeFeatureInGiniPath(f);
            if(model->selected_features.size() >= maxNbFeatures){
                break;
            }
        }
    }
}

void fit(SGDRegressor* model){
    std::cout << std::endl << "Fit Model for " << model->config->nbFeaturesInModel
                           << " variables :" << std::endl;

    long i = 0;
    double stopCriterion = model->config->loss == "poisson" ? 0.00001 : 0.000001;
    fitUntilConvergence(model, i, 1, stopCriterion);
    model->printResults();
    backwardStepwise(model, i);

    int maxSortedFeatures = model->config->p;
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
    fitUntilConvergence(model, i,  5, stopCriterion);
}

int main(int argc, char** argv){
    if(argc != 2){
        std::cout << "Invalid parameters. Expecting 1 parameter : [config file]." <<std::endl;
        return 1;
    }
    std::string config_filename = argv[1];

    Config config(config_filename, 0.2);
    Dataset ds(&config);
    SGDRegressor* model = new SGDRegressor(&config, &ds);
    fit(model);

    std::cout << std::endl
              << "Final results :" << std::endl
              << "---------------" << std::endl;

    model->printResults();
    model->printSelectedFeatures();
    model->writeResults(ds.getSample());

    std::cout << std::endl << "Finished OK." << std::endl;
    delete model;
}
