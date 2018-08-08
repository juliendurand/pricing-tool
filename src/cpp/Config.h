#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <vector>


// Configuration for the regression model. The data is read from a
// configuration file on disk.
//
// Params :
//      - name : configuration filename in filesystem ;
//
// Usage : all members are public in order to provide easy access to all the
//         stored parameters.

class Config
{
public:
    const std::string name; // configuration filename in filesystem.
    const double testPct; // % of test data to keep in traning dataset.
    std::string label; // Name of the model.
    std::string path; // Dataset path on filesystem.
    std::string resultPath; // Path use to persist results in filesystem.
    std::string target; // Name of the target feature.
    std::string weight; // Name of the exposure/weight feature.
    std::string loss; // type of model (gaussian, poisson, gamma)
    int nbFeaturesInModel;
    int n; // number of observations.
    int p; // number of features.
    int m; // total number of modalities.
    int trainSize; // nb of observation in train set.
    int testSize; // nb of observation in test set.
    std::vector<std::string> features; // names of features.
    std::vector<std::string> modalities; // names of modalities.
    std::vector<int> offsets; // index of the first modality of every features.
    std::vector<std::string> excludedFeatures; // List of excluded features.

    Config(const std::string& filename);
    std::string getFeatureFilename();
    std::string getWeightFilename();
    std::string getTargetFilename();
    std::string getTrainFilename();
    std::string getTestFilename();
    int getFeatureIndex(const std::string& feature);
    int getFeatureFromModality(int m);
};

#endif  // CONFIG_H_
