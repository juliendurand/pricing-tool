#include "Dataset.h"

#include <iostream>


Dataset::Dataset(Config* config):
    x_data(config->getFeatureFilename(), config->p, config->n, true),
    weight_data(config->getWeightFilename(), 1, config->n, true),
    y_data(config->getTargetFilename(), 1, config->n, true)
{
    std::random_device rd;
    generator = std::mt19937(rd());

    Array<int32_t> train_index(config->getTrainFilename(), 1,
        config->trainSize, true);
    std::vector<int32_t> train_data(train_index.getData(),
        train_index.getData() + config->trainSize);

    const std::size_t testSize = static_cast<std::size_t>(
        config->trainSize * config->testPct);
    std::shuffle(train_data.begin(), train_data.end(), generator);

    train = std::vector<int32_t>(train_data.begin(),
        train_data.end() - testSize);
    std::sort(train.begin(), train.end());

    test = std::vector<int32_t>(train_data.end() - testSize, train_data.end());
    std::sort(test.begin(), test.end());

    Array<int32_t> sample_index(config->getTestFilename(), 1,
        config->testSize, true);
    std::vector<int32_t> sample_data(sample_index.getData(),
        sample_index.getData() + config->testSize);
    sample.swap(sample_data);

    if(config->loss == "gamma"){
        filterNonZeroTarget();
    }

    random = std::uniform_int_distribution<std::mt19937::result_type>(0,
        train.size() - 1);

    std::cout << "# Train observations : " << train.size() << std::endl;
    std::cout << "# Test observations : " << test.size() << std::endl;
    std::cout << "# Sample observations : " << sample.size() << std::endl;
}

int Dataset::getSize(){
    return train.size();
}

int Dataset::next(){
    return train[random(generator)];
}

uint8_t* Dataset::get_x(){
    return x_data.getData();
}

float* Dataset::get_weight(){
    return weight_data.getData();
}

float* Dataset::get_y(){
    return y_data.getData();
}

std::vector<int>& Dataset::getTrain(){
    return train;
}

std::vector<int>& Dataset::getTest(){
    return test;
}

std::vector<int>& Dataset::getSample(){
    return sample;
}

void Dataset::filterNonZeroTarget(){
    float* w = get_weight();

    auto filter = [w](int i){
        return w[i] == 0;
    };

    auto end_train = std::remove_if(train.begin(), train.end(), filter);
    train.erase(end_train, train.end());

    auto end_test = std::remove_if(test.begin(), test.end(), filter);
    test.erase(end_test, test.end());

    auto end_sample = std::remove_if(sample.begin(), sample.end(), filter);
    sample.erase(end_sample, sample.end());
}
