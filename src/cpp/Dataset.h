#ifndef DATASET_H_
#define DATASET_H_

#include <random>
#include <vector>

#include "Array.h"
#include "Config.h"

// Contains the train, test and sample datasets. When using a gamma
// loss function the data is filtered to exclude all observations with
// a weight equals to 0.
//
// Params :
//      - config : the regression configuration.
//
// Usage : the public methods provide acces to the regression data, with
//         all types of data (observations, weight, target) and dataset (train,
//         test, sample).

class Dataset
{
public:
    Dataset(Config* config);
    int getSize();
    int next(); // provide a random observation from the train set.
    uint8_t* get_x(); // observations.
    float* get_weight(); // weight or exposure.
    float* get_y(); // target
    std::vector<int>& getTrain(); // list of indices for train set
    std::vector<int>& getTest(); // list of indices for test set
    std::vector<int>& getSample(); // list of indices for sample set
    void filterNonZeroWeight(); // exclude all zero weighted observations.

private:
    std::mt19937 generator;
    std::uniform_int_distribution<std::mt19937::result_type> random;
    Array<uint8_t> x_data;
    Array<float> weight_data;
    Array<float> y_data;
    std::vector<int> train;
    std::vector<int> test;
    std::vector<int> sample;
};

#endif  // DATASET_H_
