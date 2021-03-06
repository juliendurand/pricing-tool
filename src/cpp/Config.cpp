#include "Config.h"

#include <algorithm>
#include <fstream>
#include <iostream>


Config::Config(const std::string& name) :
    name(name), testPct(0.2)
{
    std::istream* cfgfile = &std::cin;
    std::ifstream cfgfilestream;

    if(name == ""){
        std::cout << "Loading config file from standard input stream. " << std::endl;
    } else {
        std::cout << "Loading config file : " << name << std::endl;
        cfgfilestream = std::ifstream(name);
        cfgfile = &cfgfilestream;
    }

    std::getline(*cfgfile, label);
    *cfgfile >> path;
    if(strcmp(&path.back(), "/") != 0){
        path += "/";
    }

    *cfgfile >> resultPath;
    resultPath = "result/" + resultPath + "/";
    std::cout << "resultPath : " << resultPath << std::endl;
    *cfgfile >> loss;
    *cfgfile >> target;
    *cfgfile >> weight;
    *cfgfile >> nbFeaturesInModel;

    std::string f;
    while(*cfgfile){
        std::getline(*cfgfile, f);
        if(f.empty()){
            continue;
        }
        excludedFeatures.push_back(f);
    }

    std::ifstream datasetfile(this->path  + "dataset.cfg");
    datasetfile >> n;
    datasetfile >> trainSize;
    datasetfile >> testSize;
    datasetfile >> p;
    datasetfile >> m;

    std::getline(datasetfile, f); // this is needed !!! (consume \n ?)
    for(int i = 0; i < p; i++){
        std::getline(datasetfile, f);
        features.push_back(f);
    }

    for(int i = 0; i < m; i++){
        std::getline(datasetfile, f);
        modalities.push_back(f);
    }

    int k;
    for(int i = 0; i < p + 1; i++){
        datasetfile >> k;
        offsets.push_back(k + 1);
    }
}

std::string Config::getFeatureFilename()
{
    return path + "features.dat";
}

std::string Config::getWeightFilename()
{
    return path + "column_" + weight + ".dat";
}

std::string Config::getTargetFilename()
{
    return path + "column_" + target + ".dat";
}

std::string Config::getTrainFilename()
{
    return path + "train.dat";
}

std::string Config::getTestFilename()
{
     return path + "test.dat";
}

// Returns the feature index from the feature name
int Config::getFeatureIndex(const std::string& feature)
{
    auto it = std::find(features.begin(), features.end(),
                         feature);
    if(it == features.end()){
        return -1;
    }
    auto idx = std::distance(features.begin(), it);
    return idx;
}
