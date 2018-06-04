#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <vector>

class Config
{
public:
    std::string label;
    std::string path;
    std::string name;
    std::string resultPath;
    std::string target;
    std::string weight;
    std::string loss;
    int nbFeaturesInModel;
    int n;
    int p;
    int m;
    int trainSize;
    int testSize;
    std::vector<std::string> features;
    std::vector<std::string> modalities;
    std::vector<int> offsets;
    std::vector<std::string> excludedFeatures;

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
