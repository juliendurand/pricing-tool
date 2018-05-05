#include <iostream>

#include "config.h"
#include "metrics.h"
#include "CDPoissonRegressor.h"
#include "SGDPoissonRegressor.h"


ALinearRegressor* fitSGD(Config* config, Dataset* ds){
    SGDPoissonRegressor* model = new SGDPoissonRegressor(config, ds);

    std::cout << std::endl << "Fit Model for " << config->nbFeaturesInModel
                           << " variables with " << config->nbIterations
                           << " iterations :" << std::endl;

    int blocksize = 10 * config->m;
    double alpha = 0.1;
    double * previousCoeffs = new double[config->m + 1];
    for(int i = 0; i < config->nbIterations / blocksize; i++){
        model->blockfit(blocksize, alpha);
        //model->penalizeRidge(alpha, 0.005);
        if(i % 100 == 0){
            std::vector<float> ypred = model->predict();
            std::cout << i * blocksize << "th iteration : ";
            LinearRegressionResult(model).print(ds->train, ds->test);
            std::cout << "Diff of coeffs : " << model->getNorm2CoeffDiff(previousCoeffs)
                      << std::endl;
        }

        if(i > 3000 && model->selected_features.size() > config->nbFeaturesInModel){
            if(model->selected_features.size() > config->nbFeaturesInModel + 20){
            int remove_feature = model->getMinCoeff(model->selected_features);
            model->selected_features.erase(remove_feature);
            model->eraseFeature(i * blocksize, remove_feature);
            } else if(i % 100 == 0){
                int remove_feature = model->getMinCoeff(model->selected_features);
                model->selected_features.erase(remove_feature);
                model->eraseFeature(i * blocksize, remove_feature);
            }
        }

        if(model->selected_features.size() == config->nbFeaturesInModel){
            alpha = 0.1;
        }

        for(int j = 0; j < config->m + 1; j++){
            previousCoeffs[j] = model->coeffs[j];
        }
    }
    return model;
}

ALinearRegressor* fitGammaSGD(Config* config, Dataset* ds){
    ds->filterNonZeroTarget();
    std::cout << ds->train.size() << std::endl;
    SGDPoissonRegressor* model = new SGDPoissonRegressor(config, ds);

    std::cout << std::endl << "Fit Model for " << config->nbFeaturesInModel
                           << " variables with " << config->nbIterations
                           << " iterations :" << std::endl;

    int blocksize = 10 * config->m;
    double alpha = 0.01;
    double * previousCoeffs = new double[config->m + 1];
    for(int i = 0; i < config->nbIterations / blocksize; i++){
        model->fitGamma(blocksize, alpha);
        //model->penalizeRidge(alpha, 0.005);
        if(i % 100 == 0){
            std::vector<float> ypred = model->predict();
            std::cout << i * blocksize << "th iteration : ";
            LinearRegressionResult(model).print(ds->train, ds->test);
            std::cout << "Diff of coeffs : " << model->getNorm2CoeffDiff(previousCoeffs)
                      << std::endl;
        }

        if(i > 100  && model->selected_features.size() > config->nbFeaturesInModel){
            if(model->selected_features.size() > config->nbFeaturesInModel + 20){
            int remove_feature = model->getMinCoeff(model->selected_features);
            model->selected_features.erase(remove_feature);
            model->eraseFeature(i * blocksize, remove_feature);
            } else if(i % 5 == 0){
                int remove_feature = model->getMinCoeff(model->selected_features);
                model->selected_features.erase(remove_feature);
                model->eraseFeature(i * blocksize, remove_feature);
            }
        }

        if(model->selected_features.size() == config->nbFeaturesInModel){
            //alpha = 0.1;
        }

        for(int j = 0; j < config->m + 1; j++){
            previousCoeffs[j] = model->coeffs[j];
        }
    }
    return model;
}

ALinearRegressor* fitCD(Config* config, Dataset* ds){
    CDPoissonRegressor* model = new CDPoissonRegressor(config, ds);

    std::cout << std::endl << "Fit Model for " << config->nbFeaturesInModel
                           << " variables with " << config->nbIterations
                           << " iterations :" << std::endl;

    int blocksize = 10 * config->m;
    double alpha = 0.2;
    double * previousCoeffs = new double[config->m + 1];
    for(int i = 0; i < config->nbIterations; i++){
        model->fit(blocksize*100, alpha);
        if(i % 1 == 0){
            LinearRegressionResult(model).print(ds->train, ds->test);
        }
        if(i == 6){
            while(model->selected_features.size() > config->nbFeaturesInModel){
                int remove_feature = model->getMinCoeff(model->selected_features);
                model->selected_features.erase(remove_feature);
                model->eraseFeature(i * blocksize, remove_feature);
            }
        }
        if(i == 12) break;
    }

    return model;

    for(int i = 0; i < config->nbIterations / blocksize; i++){
        model->fit(blocksize * 10, alpha);
        //model->penalizeRidge(alpha, 0.005);
        if(i % 100 == 0){
            std::vector<float> ypred = model->predict();
            std::cout << i * blocksize << "th iteration : ";
            LinearRegressionResult(model).print(ds->train, ds->test);
            std::cout << "Diff of coeffs : " << model->getNorm2CoeffDiff(previousCoeffs)
                      << std::endl;
        }

        if(i > 3000 && model->selected_features.size() > config->nbFeaturesInModel){
            if(model->selected_features.size() > config->nbFeaturesInModel + 20){
            int remove_feature = model->getMinCoeff(model->selected_features);
            model->selected_features.erase(remove_feature);
            model->eraseFeature(i * blocksize, remove_feature);
            } else if(i % 100 == 0){
                int remove_feature = model->getMinCoeff(model->selected_features);
                model->selected_features.erase(remove_feature);
                model->eraseFeature(i * blocksize, remove_feature);
            }
        }

        if(model->selected_features.size() == config->nbFeaturesInModel){
            alpha = 0.1;
        }

        for(int j = 0; j < config->m + 1; j++){
            previousCoeffs[j] = model->coeffs[j];
        }
    }

    return model;
}


int main(int argc, char** argv){
    if(argc != 3){
        std::cout << "Invalid parameters. Expecting 2 parameters : [path] [config file]." <<std::endl;
        return 1;
    }

    char* path = argv[1];
    char* config_filename = argv[2];

    Config config(path, config_filename);

    Dataset ds(&config, 0.2);

    ALinearRegressor* model = fitGammaSGD(&config, &ds);

    std::cout << std::endl << "Final results :" << std::endl
              << "---------------" << std::endl;
    LinearRegressionResult(model).print(ds.train, ds.test);
    model->printSelectedFeatures();
    std::cout << std::endl;
    model->writeResults("./data/results.csv", ds.test);
    std::cout << "Finished OK." << std::endl;
}
