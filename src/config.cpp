#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "config.h"

void Config::load(const std::string& path, const std::string& name){
    this->path = path + "/" ;
    this->name = name;

    std::cout << "Loading config file : " << this->path + name + ".cfg" <<std::endl;


    std::ifstream cfgfile(this->path + name + ".cfg");
    cfgfile >> target;
    cfgfile >> loss;
    cfgfile >> nbFeaturesInModel;

    std::ifstream metadatafile(this->path  + "metadata.cfg");
    metadatafile >> n;
    metadatafile >> p;
    metadatafile >> m;

    std::string f;
    std::getline(metadatafile, f); // this is needed !!! (consume \n ?)
    for(int i = 0; i < p; i++){
        std::getline(metadatafile, f);
        features.push_back(f);
    }

    for(int i = 0; i < m; i++){
        std::getline(metadatafile, f);
        modalities.push_back(f);
    }

    int k;
    for(int i = 0; i < p + 1; i++){
        metadatafile >> k;
        offsets.push_back(k);
    }

    std::ifstream excludedFeatureFile(this->path  + name + "_exclude.cfg");
    while(excludedFeatureFile){
        excludedFeatureFile >> f;
        excludedFeatures.push_back(f);
    }

    std::cout << "Loading config file : OK. " << std::endl;
}

std::string Config::getFeatureFilename(){
    return path + "features.dat";
}

std::string Config::getExposureFilename(){
    return path + "exposure.dat";
}

std::string Config::getTargetFilename(){
    return path + "target_" + target + ".dat";
}

int Config::getFeatureIndex(const std::string& feature){
    auto it = std::find(features.begin(), features.end(),
                         feature);
    if(it == excludedFeatures.end()){
        throw std::invalid_argument( "ERROR : Excluded feature " + feature +
                                    " can not be found." );
    }
    auto idx = std::distance(features.begin(), it);
    return idx;
}
