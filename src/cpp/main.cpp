#include <iostream>

#include "config.h"
#include "SGDPoissonRegressor.h"


void fitToConvergence(ALinearRegressor* model, long& i,
                      int blocksize, double alpha, int precision){
    double minll = 1e30;
    int nbIterationsSinceMinimum = 0;
    int epoch = model->dataset->train.size() / blocksize;
    double sg = 0;
    for(; nbIterationsSinceMinimum < precision; i++){
        sg = model->fit(blocksize, alpha, 0);
        if(i % epoch == 0){
            std::cout << i * blocksize << "th iteration : " << " minll " << minll << " iteration since min " << nbIterationsSinceMinimum << std::endl;
            model->printResults();
            model->predict(model->dataset->train);
            double ll = model->logLikelihood(model->dataset->train);
            if(ll < minll - 0.00001) {
                minll = ll;
                nbIterationsSinceMinimum = 0;
            } else {
                nbIterationsSinceMinimum++;
            }
        }
    }
}

void backwardStepwise(ALinearRegressor* model, long& i,
                     int blocksize, double alpha){
    std::cout << "Backward Stepwise" << std::endl;
    int epoch = model->dataset->train.size() / blocksize;
    for(;; i++){
        model->fit(blocksize, alpha, 0);
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

void forwardStepwise(ALinearRegressor* model, long& i,
                     int blocksize, double alpha, int maxNbFeatures){
    std::cout << "Forward Stepwise" << std::endl;
    model->eraseAllFeatures();
    int epoch = model->dataset->train.size() / blocksize;
    int k = 0;
    for(auto p : model->giniPath){
        int f = p.feature_idx;
        if(f >= 0){
            model->addFeatures({f});
            while(++i % epoch != 0){
                model->fit(blocksize, alpha, 0);
            }
            model->storeFeatureInGiniPath(f);
            if(++k >= maxNbFeatures){
                break;
            }
        }
    }
}

ALinearRegressor* fit(Config* config, Dataset* ds){
    SGDPoissonRegressor* model = new SGDPoissonRegressor(config, ds);
    std::cout << std::endl << "Fit Model for " << config->nbFeaturesInModel
                           << " variables :" << std::endl;

    int blocksize = 200;
    double alpha = 0.0001;
    long i = 0;
    fitToConvergence(model, i, blocksize, alpha, 1);
    model->printResults();
    backwardStepwise(model, i, blocksize, alpha);

    int maxSortedFeatures = config->p;
    for(int k = 0; k < 6; k++){
        std::vector<int> bestFeatures = model->getBestFeatures(maxSortedFeatures, 0.0001);
        maxSortedFeatures = bestFeatures.size();
        forwardStepwise(model, i, blocksize, alpha, maxSortedFeatures);
    }
    maxSortedFeatures = std::min(maxSortedFeatures, model->config->nbFeaturesInModel);
    std::vector<int> bestFeatures = model->getBestFeatures(maxSortedFeatures, 0.0002);
    forwardStepwise(model, i, blocksize, alpha, maxSortedFeatures);
    model->eraseAllFeatures();
    model->addFeatures(bestFeatures);
    fitToConvergence(model, i, blocksize, alpha, 5);

    return model;
}

int main(int argc, char** argv){
    if(argc != 2){
        std::cout << "Invalid parameters. Expecting 1 parameter : [config file]." <<std::endl;
        return 1;
    }
    std::string config_filename = argv[1];

    Config config(config_filename);
    Dataset ds(&config, 0.2);
    ALinearRegressor* model = fit(&config, &ds);

    std::cout << std::endl
              << "Final results :" << std::endl
              << "---------------" << std::endl;

    model->printResults();
    model->printSelectedFeatures(model->selected_features.size());
    model->writeResults(ds.sample);

    std::cout << std::endl << "Finished OK." << std::endl;
}
