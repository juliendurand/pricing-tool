#include "Dataset.h"

#include <iostream>


// Creates the train, test and sample dataset
Dataset::Dataset(Config* config):
    x_data(config->getFeatureFilename(), config->p, config->n),
    weight_data(config->getWeightFilename(), 1, config->n),
    y_data(config->getTargetFilename(), 1, config->n)
{
    // initializes a random number generator
    std::random_device rd;
    generator = std::mt19937(rd());

    // loads the train set indices
    Array<int32_t> train_index(config->getTrainFilename(), 1,
        config->trainSize);
    std::vector<int32_t> train_data(train_index.getData(),
        train_index.getData() + config->trainSize);

    // randomizes the train set to create random train and test
    std::shuffle(train_data.begin(), train_data.end(), generator);

   const std::size_t testSize = static_cast<std::size_t>(
        config->trainSize * config->testPct);

    // creates the final train set and reorder indices
    train = std::vector<int32_t>(train_data.begin(),
        train_data.end() - testSize);
    std::sort(train.begin(), train.end());

    // creates the final test set and reorder indices
    test = std::vector<int32_t>(train_data.end() - testSize, train_data.end());
    std::sort(test.begin(), test.end());

    // loads the sample set indices
    Array<int32_t> sample_index(config->getTestFilename(), 1,
        config->testSize);
    std::vector<int32_t> sample_data(sample_index.getData(),
        sample_index.getData() + config->testSize);
    sample.swap(sample_data);

    // excludes all zero weighted observation.
    filterNonZeroWeight();

    // uniform random number generator for the next() function
    random = std::uniform_int_distribution<std::mt19937::result_type>(0,
        train.size() - 1);

    std::cout << "# Train observations : " << train.size() << std::endl;
    std::cout << "# Test observations : " << test.size() << std::endl;
    std::cout << "# Sample observations : " << sample.size() << std::endl;
}

int Dataset::getSize()
{
    return train.size();
}

// randomly pick an observation in the train set
int Dataset::next()
{
    return train[random(generator)];
}

uint8_t* Dataset::get_x()
{
    return x_data.getData();
}

float* Dataset::get_weight()
{
    return weight_data.getData();
}

float* Dataset::get_y()
{
    return y_data.getData();
}

std::vector<int>& Dataset::getTrain()
{
    return train;
}

std::vector<int>& Dataset::getTest()
{
    return test;
}

std::vector<int>& Dataset::getSample()
{
    return sample;
}

void Dataset::filterNonZeroWeight()
{
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
