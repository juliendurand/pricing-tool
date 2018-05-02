#include <iostream>

#include "config.h"
#include "metrics.h"
#include "SGDPoissonRegressor.h"


int main(int argc, char** argv){
    if(argc != 3){
        std::cout << "Invalid parameters. Expecting 2 parameters : [path] [config file]." <<std::endl;
        return 1;
    }

    char* path = argv[1];
    char* config_filename = argv[2];

    Config config(path, config_filename);

    Dataset ds(&config, 0.2);

    SGDPoissonRegressor model(&config, &ds);

    std::cout << std::endl << "Fit Model for " << config.nbFeaturesInModel << " variables..." << std::endl;
    int nb_iterations = 20000000;

    int blocksize = 10 * config.m;
    double alpha = 0.01;
    for(int i=0; i < nb_iterations / blocksize; i++){
        model.blockfit(blocksize, alpha);
        //model.penalizeRidge(alpha, 0.005);
        if(i % 1000 == 0){
            std::vector<float> ypred = model.predict();
            std::cout << i * blocksize << "th iteration : ";
            LinearRegressionResult(model).print(ds.train, ds.test);
        }

        if(i > 3000 && model.selected_features.size() > config.nbFeaturesInModel){
            if(model.selected_features.size() > config.nbFeaturesInModel + 20){
            int remove_feature = model.getMinCoeff(model.selected_features);
            model.selected_features.erase(remove_feature);
            model.eraseFeature(i * blocksize, remove_feature);
            } else if(i % 100 == 0){
                int remove_feature = model.getMinCoeff(model.selected_features);
                model.selected_features.erase(remove_feature);
                model.eraseFeature(i * blocksize, remove_feature);
            }
        }

        if(model.selected_features.size() == config.nbFeaturesInModel){
            alpha = 0.1;
        }
    }

    std::cout << std::endl << "Final results :" << std::endl
              << "---------------" << std::endl;
    LinearRegressionResult(model).print(ds.train, ds.test);
    model.printSelectedFeatures();
    std::cout << std::endl;
    model.writeResults("./data/results.csv", ds.test);
    std::cout << "Finished OK." << std::endl;
}
