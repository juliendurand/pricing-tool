#ifndef DATASET_H_
#define DATASET_H_

#include <random>
#include <vector>

#include "array.h"
#include "config.h"


class Dataset
{
private:
    float testPct;
    std::mt19937 generator;
    std::uniform_int_distribution<std::mt19937::result_type> random;
    Array<uint8_t>* x_data;
    Array<float>* weight_data;
    Array<float>* y_data;
    Array<int32_t>* train_index;
    Array<int32_t>* sample_index;

public:
    std::vector<int> train;
    std::vector<int> test;
    std::vector<int> sample;

    Dataset(Config* config, float testPct);
    int next();
    void filterNonZeroTarget();
    uint8_t* get_x();
    float* get_weight();
    float* get_y();
};

#endif  // DATASET_H_
