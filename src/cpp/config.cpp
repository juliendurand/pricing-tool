#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "config.h"

Config::Config(const std::string& name) : name(name) {
    std::cout << "Loading config file : " << name <<std::endl;
    std::ifstream cfgfile(name);
    cfgfile >> path;
    if(strcmp(&path.back(), "/") != 0){
        path += "/";
    }
    cfgfile >> resultPath;
    resultPath = "result/" + resultPath + "/";
    std::cout << "resultPath : " << resultPath << std::endl;
    cfgfile >> loss;
    cfgfile >> target;
    cfgfile >> weight;
    cfgfile >> nbFeaturesInModel;

    std::ifstream datasetfile(this->path  + "dataset.cfg");
    datasetfile >> n;
    datasetfile >> trainSize;
    datasetfile >> testSize;
    datasetfile >> p;
    datasetfile >> m;

    std::string f;
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
        offsets.push_back(k);
    }

    std::ifstream excludedFeatureFile(this->path  + name + "_exclude.cfg");
    while(excludedFeatureFile){
        std::getline(excludedFeatureFile, f);
        if(f.empty()){
            continue;
        }
        std::cout << f << " f" << std::endl;
        excludedFeatures.push_back(f);
    }
}

std::string Config::getFeatureFilename(){
    return path + "features.dat";
}

std::string Config::getWeightFilename(){
    return path + "column_" + weight + ".dat";
}

std::string Config::getTargetFilename(){
    return path + "column_" + target + ".dat";
}

std::string Config::getTrainFilename(){
    return path + "train.dat";
}

std::string Config::getTestFilename(){
     return path + "test.dat";
}

int Config::getFeatureIndex(const std::string& feature){
    auto it = std::find(features.begin(), features.end(),
                         feature);
    std::cout << feature << " " << std::endl;
    if(it == features.end()){
        throw std::invalid_argument("ERROR : Excluded feature " + feature +
                                    " can not be found." );
    }
    auto idx = std::distance(features.begin(), it);
    return idx;
}

int Config::getFeatureFromModality(int m){
    for(int i = 0; i < features.size(); i++){
        if(m >= offsets[i] && m < offsets[i + 1]){
            return i;
        }
    }
    throw std::invalid_argument("ERROR : Modality " + std::to_string(m) +
                                " can not be found." );
}
