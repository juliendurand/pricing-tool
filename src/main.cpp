
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

    //int p = 134;
    //int n = 4459543;
    Dataset ds(config.n, 0.2);

    Array<uint8_t> x_data(config.getFeatureFilename(), config.p, config.n);
    uint8_t* x = x_data.getData();
    Array<float> exposure_data(config.getExposureFilename(), 1, config.n * 4);
    float* exposure = exposure_data.getData();
    Array<float> y_data(config.getTargetFilename(), 1, config.n * 4);
    float* y = y_data.getData();

    /*std::vector<int> nbModalities = {
        7, 7, 2, 7, 7, 45, 7, 3, 6, 2, 3, 11, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 6, 16, 25, 28, 34, 6, 31, 131, 5, 51, 47, 27, 22, 34, 51, 16, 30, 13, 11, 16, 14, 15, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 7, 2, 2, 2, 2, 2, 11, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 9, 10, 10, 9, 25, 6, 12, 12, 17, 10, 4, 4, 7, 1, 2, 2, 2, 81, 21, 11, 21, 21, 21, 21, 21, 21, 21, 11, 21, 30, 4, 4, 8, 21, 16, 21
    };

    std::vector<int> offsets(nbModalities.size() + 1);
    for(int i = 1; i < nbModalities.size() + 1; i++){
        offsets[i] = offsets[i - 1] + nbModalities[i - 1];
    }
    int nbTotalModalities = offsets.back();
    */

    std::set<int> selected_features;
    for(int i=0; i < config.p; i++){
        selected_features.insert(i);
    }

    SGDPoissonRegressor model(config.p, config.n, x, y, exposure, config.m, config.offsets, config.features);

    std::cout << "Fit Model..." << std::endl;
    int nb_iterations = 100000000;
    int nbMaxSelectedFeatures = 12;

    for(int i=0; i < nb_iterations; i++){
        double alpha = 0.01;

        model.filterfit(ds.next(), alpha, selected_features);
        if(selected_features.size() <= nbMaxSelectedFeatures){
            //model.penalizeRidge(alpha, 0.001);
        }

        //int non_zero = model.penalizeLasso(alpha, 0.01);
        if(i % 10000000 == 0){
            std::vector<float> ypred = model.predict();
            LinearRegressionResult res(config.p, config.n, x, y, ypred, exposure, model.coeffs);
            std::cout << i << " : rmse train=" << res.rmse(ds.train) << ", test=" << res.rmse(ds.test) << ")" << " | gini(train=" << res.gini(ds.train) << ", test=" << res.gini(ds.test) << ")" << std::endl;
            //std::cout << non_zero << std::endl;
        }
        if(i > 50000000 &&  i % 300000 == 0 && selected_features.size() > nbMaxSelectedFeatures){
            int remove_feature = model.getMinCoeff(selected_features);
            std::cout << "Removing: " << config.features[remove_feature] << " Norm2=" << model.getCoeffNorm2(remove_feature) << std::endl;
            selected_features.erase(remove_feature);
            for(int j = config.offsets[remove_feature]; j < config.offsets[remove_feature + 1]; j++){
                model.coeffs[j + 1] = 0;
            }
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
    LinearRegressionResult result(config.p, config.n, x, y, ypred, exposure, model.coeffs);
    std::cout << "rmse (train) : " << result.rmse(ds.train) << std::endl;
    std::cout << "rmse (test) : " << result.rmse(ds.test) << std::endl;
    std::cout << "gini (train) : " << result.gini(ds.train) << std::endl;
    std::cout << "gini (test) : " << result.gini(ds.test) << std::endl;
/*
    int blocksize = 10000;
    for(int i=0; i < nb_iterations / blocksize; i++){

        model.blockfit(ds, 10000, 0.5);
        if((i * blocksize ) % 10000000 == 0){
            std::vector<float> ypred = model.predict();
            LinearRegressionResult res(p, n, x, y, ypred, exposure, model.coeffs);
            std::cout << i * blocksize << " : rmse train=" << res.rmse(ds.train) << ", test=" << res.rmse(ds.test) << ")" << " | gini(train=" << res.gini(ds.train) << ", test=" << res.gini(ds.test) << ")" << std::endl;
        }
    }


    ypred = model.predict();
    result = LinearRegressionResult(p, n, x, y, ypred, exposure, model.coeffs);
    std::cout << "rmse (train) : " << result.rmse(ds.train) << std::endl;
    std::cout << "rmse (test) : " << result.rmse(ds.test) << std::endl;
    std::cout << "gini (train) : " << result.gini(ds.train) << std::endl;
    std::cout << "gini (test) : " << result.gini(ds.test) << std::endl;
*/
    //model.printGroupedCoeffN2();

    model.writeResults("./data/results.csv", ds.test);

    std::cout << "Finished OK." << std::endl;
}
