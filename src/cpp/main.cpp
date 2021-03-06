#include <iostream>

#include "FeatureSelector.h"
#include "SGDRegressor.h"


int main(int argc, char** argv)
{
    if(argc > 2){
        std::cout
            << "Invalid parameters. Expecting 0 (stdin) or 1 parameter : [config file]."
            << std::endl;
        return 1;
    }

    std::string config_filename = "";
    if(argc == 2){
        config_filename = argv[1];
    }
    Config config(config_filename); // Loads config file.
    Dataset ds(&config); // Loads dataset.

    SGDRegressor model(&config, &ds); // Creates the regression engine.
    FeatureSelector fs(&model); // Creates the feature selection engine.

    fs.fit(); // Performs all the calculations.

    fs.printSelectedFeatures(); // Prints final results.

    // Saves results to disk.
    fs.saveResults();
    auto coeffs = model.getCoeffs();
    coeffs->saveResults();
    auto result = coeffs->predict(&ds, ds.getSample());

    std::cout << std::endl << "Saving results." << std::endl;
    result->saveResults();

    std::cout << std::endl << "Finished OK." << std::endl;
}
