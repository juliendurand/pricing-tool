#include <iostream>

#include "dataset.h"

Dataset::Dataset(){
}

Dataset::Dataset(Config* config, float testPercent){
    std::random_device rd;
    generator = std::mt19937(rd());

    x_data = new Array<uint8_t>(config->getFeatureFilename(), config->p, config->n);
    weight_data = new Array<float>(config->getWeightFilename(), 1, config->n * 4);
    y_data = new Array<float>(config->getTargetFilename(), 1, config->n * 4);

    train_index = new Array<int32_t>(config->getTrainFilename(), 1, config->trainSize * 4);
    std::vector<int32_t> train_data(train_index->getData(), train_index->getData() + config->trainSize);

    testPct = testPercent;
    std::size_t const testSize = static_cast<std::size_t>(config->trainSize * testPct);
    std::shuffle(train_data.begin(), train_data.end(), generator);

    train = std::vector<int32_t>(train_data.begin(), train_data.end() - testSize);
    std::sort(train.begin(), train.end());

    test = std::vector<int32_t>(train_data.end() - testSize, train_data.end());
    std::sort(test.begin(), test.end());

    sample_index = new Array<int32_t>(config->getTestFilename(), 1, config->testSize * 4);
    std::vector<int32_t> sample_data(sample_index->getData(), sample_index->getData() + config->testSize);
    sample.swap(sample_data);

    if(config->loss == "gamma"){
        filterNonZeroTarget();
    }

    std::cout << "# Train observations : " << train.size() << std::endl;
    std::cout << "# Test observations : " << test.size() << std::endl;
    std::cout << "# Sample observations : " << sample.size() << std::endl;

    random = std::uniform_int_distribution<std::mt19937::result_type>(0, train.size() - 1);
}

int Dataset::next(){
    return train[random(generator)];
}

void Dataset::filterNonZeroTarget(){
    float* w = weight_data->getData();

    auto end_train = std::remove_if(train.begin(), train.end(),
                                    [w](int i){return w[i] == 0;});
    train.erase(end_train, train.end());

    auto end_test = std::remove_if(test.begin(), test.end(),
                                   [w](int i){return w[i] == 0;});
    test.erase(end_test, test.end());

    auto end_sample = std::remove_if(sample.begin(), sample.end(),
                                     [w](int i){return w[i] == 0;});
    sample.erase(end_sample, sample.end());
}
