#include <iostream>

#include "FeatureSelector.h"
#include "SGDRegressor.h"


int main(int argc, char** argv){
    if(argc != 2){
        std::cout
            << "Invalid parameters. Expecting 1 parameter : [config file]."
            << std::endl;
        return 1;
    }

    std::string config_filename = argv[1];
    Config config(config_filename);
    Dataset ds(&config);

    SGDRegressor model(&config, &ds);
    FeatureSelector fs(&model);
    fs.fit();

    std::cout << std::endl
              << "Final results :" << std::endl
              << "---------------" << std::endl;
    fs.printSelectedFeatures();
    model.printResults();
    model.writeResults(ds.getSample());

    std::cout << std::endl << "Finished OK." << std::endl;
}
