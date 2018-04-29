#include <fstream>
#include <iostream>

#include "config.h"

void Config::load(const std::string& path, const std::string& filename){
    this->path = path + "/" ;
    name = filename;

    std::cout << "Loading config file : " << path + filename <<std::endl;


    std::ifstream cfgfile(this->path + filename);
    cfgfile >> target;
    cfgfile >> loss;

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
