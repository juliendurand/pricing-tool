#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <vector>

class Config
{
public:
    std::string path;
    std::string name;
    std::string target;
    std::string loss;
    int nbFeaturesInModel;
    int nbIterations;
    int n;
    int p;
    int m;
    std::vector<std::string> features;
    std::vector<std::string> modalities;
    std::vector<int> offsets;
    std::vector<std::string> excludedFeatures;

    Config(const std::string& path, const std::string& filename);
    std::string getFeatureFilename();
    std::string getExposureFilename();
    std::string getTargetFilename();
    int getFeatureIndex(const std::string& feature);
    int getFeatureFromModality(int m);
};

#endif  // CONFIG_H_
