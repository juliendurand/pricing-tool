#include <iostream>

#include "config.h"
#include "SGDPoissonRegressor.h"


void fitToConvergence(ALinearRegressor* model, Dataset* ds, int& i, int blocksize, double alpha){
    double minll = 1e30;
    int nbIterationsSinceMinimum = 0;
    for(; nbIterationsSinceMinimum < 3; i++){
        model->fit(blocksize, alpha);
        if(i % 100 == 0){
            std::cout << i * blocksize << "th iteration : ";
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
    double alpha = 0.02;
    int i = 0;
    fitToConvergence(model, ds, i, blocksize, alpha);
    for(int nbExcessFeatures = 1; nbExcessFeatures > 0; i++){
        model->fit(blocksize, alpha);
        if(((nbExcessFeatures > 20) && (i % 10 == 0)) || (i % 100 == 0)){
            int remove_feature = model->getMinCoeff(model->selected_features);
            model->eraseFeature(i * blocksize, remove_feature);
        }
        nbExcessFeatures = model->selected_features.size() -
            config->nbFeaturesInModel;
    }
    fitToConvergence(model, ds, i, blocksize, alpha);
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

    std::cout << std::endl << "Final results :" << std::endl
              << "---------------" << std::endl;
    model->predict();
    model->printResults(ds.train, ds.test);
    model->printSelectedFeatures();
    std::cout << std::endl;
    model->writeResults("./data/results.csv", ds.test);
    std::cout << "Finished OK." << std::endl;
}
