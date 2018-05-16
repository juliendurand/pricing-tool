#include <iostream>

#include "config.h"
#include "SGDPoissonRegressor.h"


void fitToConvergence(ALinearRegressor* model, Dataset* ds, long& i,
                      int blocksize, double alpha, double l2){
    double minll = 1e30;
    int nbIterationsSinceMinimum = 0;
    for(; nbIterationsSinceMinimum < 3; i++){
        model->fit(blocksize, alpha, l2);
        if(i % 100 == 0){
            std::cout << i * blocksize << "th iteration : " << std::endl;
            model->predict();
            model->printResults(ds->train, ds->test);
            double ll = model->logLikelihood(ds->train);
            if(ll < minll) {
                minll = ll;
                nbIterationsSinceMinimum = 0;
            } else {
                nbIterationsSinceMinimum++;
            }
        }
    }
}

ALinearRegressor* fit(Config* config, Dataset* ds){
    SGDPoissonRegressor* model = new SGDPoissonRegressor(config, ds);

    std::cout << std::endl << "Fit Model for " << config->nbFeaturesInModel
                           << " variables :" << std::endl;

    int blocksize = 10 * config->m;
    double alpha = 0.03;
    long i = 0;
    float l2 = 0;
    fitToConvergence(model, ds, i, blocksize, alpha, l2);;
    for(; model->selected_features.size() > 20; i++){
        model->fit(blocksize, alpha, l2);
        if(i % 10 == 0){
            int remove_feature = model->getMinCoeff(model->selected_features);
            model->eraseFeature(i * blocksize, remove_feature);
        }
    }
    for(; model->selected_features.size() > 0; i++){
        fitToConvergence(model, ds, i, blocksize, alpha, l2);
        if(model->selected_features.size() == config->nbFeaturesInModel){
            std::cout << std::endl << "Final results :" << std::endl
                      << "---------------" << std::endl;
            model->predict();
            model->printResults(ds->train, ds->test);
            model->printSelectedFeatures();
            model->writeResults(ds->test);
            std::cout << std::endl;
        }
        int remove_feature = model->getMinCoeff(model->selected_features);
        model->eraseFeature(i * blocksize, remove_feature);
    }
    fitToConvergence(model, ds, i, blocksize, alpha, l2);
    model->eraseFeature(i * blocksize, -1);
    model->writeGiniPath();

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
    fit(&config, &ds);

    std::cout << "Finished OK." << std::endl;
}
