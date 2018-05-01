
#include <cmath>
#include <iostream>
#include <map>

#include "array.h"
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

    Config config;
    config.load(path, config_filename);

    Dataset ds(config, 0.2);

    SGDPoissonRegressor model(config, ds);

    std::set<int> selected_features;
    for(int i=0; i < config.p; i++){
        selected_features.insert(i);
    }

    for(std::string feature : config.excludedFeatures){
        int featureIdx = config.getFeatureIndex(feature);
        selected_features.erase(featureIdx);
        model.eraseFeature(0, featureIdx, config);
    }

    std::cout << "ok 3" << std::endl;

    std::cout << "Fit Model for " << config.nbFeaturesInModel << " variables..." << std::endl;
    int nb_iterations = 500000000;
/*
    for(int i=0; i < nb_iterations; i++){
        double alpha = 0.000001;

        model.filterfit(ds.next(), alpha, selected_features);
        if(selected_features.size() <= nbMaxSelectedFeatures){
            //model.penalizeRidge(alpha, 0.01);
        }

        //int non_zero = model.penalizeLasso(alpha, 0.01);
        if(i % 1000000 == 0){
            std::vector<float> ypred = model.predict();
            LinearRegressionResult res(config.p, config.n, x, y, ypred, exposure, model.coeffs);
            std::cout << i << " : rmse train=" << res.rmse(ds.train) << ", test=" << res.rmse(ds.test) << ")" << " | gini(train=" << res.gini(ds.train) << ", test=" << res.gini(ds.test) << ")" << std::endl;
            //std::cout << non_zero << std::endl;
        }
        if(i > 20000000 &&  i % 100000 == 0 && selected_features.size() > nbMaxSelectedFeatures){
            int remove_feature = model.getMinCoeff(selected_features);
            selected_features.erase(remove_feature);
            model.eraseFeature(remove_feature, config);
       }
    }
*/


    int blocksize = 10 * config.m;
    double alpha = 0.3;
    for(int i=0; i < nb_iterations / blocksize; i++){
        model.blockfit(ds, blocksize, alpha, selected_features);
        //model.penalizeRidge(alpha, 0.005);
        if(i % 1000 == 0){
            std::vector<float> ypred = model.predict();
            LinearRegressionResult res(config.p, config.n, model.x, model.y, ypred, model.exposure, model.coeffs);
            std::cout << i * blocksize << " : rmse(train=" << res.rmse(ds.train) << ", test=" << res.rmse(ds.test) << ")" << " | gini(train=" << res.gini(ds.train) << ", test=" << res.gini(ds.test) << ")" << std::endl;
        }

        if(i > 3000 && selected_features.size() > config.nbFeaturesInModel){
            if(selected_features.size() > config.nbFeaturesInModel + 20){
            int remove_feature = model.getMinCoeff(selected_features);
            selected_features.erase(remove_feature);
            model.eraseFeature(i * blocksize, remove_feature, config);
            } else if(i % 100 == 0){
                int remove_feature = model.getMinCoeff(selected_features);
                selected_features.erase(remove_feature);
                model.eraseFeature(i * blocksize, remove_feature, config);
            }
        }

        if(selected_features.size() == config.nbFeaturesInModel){
            alpha = 0.1;
        }
    }

    std::map<double, int> keep_features;
    for(auto f : selected_features){
        double v = model.getSpread(f);
        keep_features[v] = f;
    }

    for(auto kv = keep_features.rbegin(); kv != keep_features.rend(); kv++){
        std::cout << "Keep: " << config.features[kv->second] << " N2: " << model.getCoeffNorm2(kv->second) << " Spread 100/0: " << model.getSpread(kv->second) << "%" << std::endl;
    }

    std::vector<float> ypred = model.predict();
    LinearRegressionResult result(config.p, config.n, model.x, model.y, ypred, model.exposure, model.coeffs);
    std::cout << "rmse (train) : " << result.rmse(ds.train) << std::endl;
    std::cout << "rmse (test) : " << result.rmse(ds.test) << std::endl;
    std::cout << "gini (train) : " << result.gini(ds.train) << std::endl;
    std::cout << "gini (test) : " << result.gini(ds.test) << std::endl;

    std::cout << "Writting results." << std::endl;
    model.writeResults("./data/results.csv", ds.test);

    std::cout << "Finished OK." << std::endl;
}
