#ifndef DATASET_H_
#define DATASET_H_

#include <random>
#include <vector>

#include "array.h"
#include "config.h"


class Dataset
{
private:
    std::mt19937 generator;
    std::uniform_int_distribution<std::mt19937::result_type> random;
    Array<uint8_t>* x_data;
    Array<float>* weight_data;
    Array<float>* y_data;

public:
    std::vector<int> train;
    std::vector<int> test;
    std::vector<int> sample;
public:
    Dataset(Config* config);
    int getSize();
    int next();
    uint8_t* get_x();
    float* get_weight();
    float* get_y();
    void filterNonZeroTarget();
};

#endif  // DATASET_H_
