#include <iostream>

#include "dataset.h"

Dataset::Dataset(){
}

Dataset::Dataset(Config* config, float testPercent){
    std::random_device rd;
    generator = std::mt19937(rd());

    //testPct = testPercent;
    //std::vector<int> index(config->n);
    //std::iota(std::begin(index), std::end(index), 0);
    //std::shuffle(index.begin(), index.end(), generator);
    //std::size_t const testSize = static_cast<std::size_t>(config->n * testPct);
    //train = std::vector<int>(index.begin(), index.end() - testSize);
    //std::sort(train.begin(), train.end());
    //test = std::vector<int>(index.end() - testSize, index.end());
    //std::sort(test.begin(), test.end());

    x_data = new Array<uint8_t>(config->getFeatureFilename(), config->p, config->n);
    weight_data = new Array<float>(config->getWeightFilename(), 1, config->n * 4);
    y_data = new Array<float>(config->getTargetFilename(), 1, config->n * 4);

    train_index = new Array<int32_t>(config->getTrainFilename(), 1, config->trainSize * 4);
    std::vector<int32_t> train_data(train_index->getData(), train_index->getData() + config->trainSize);
    train.swap(train_data);

    test_index = new Array<int32_t>(config->getTestFilename(), 1, config->testSize * 4);
    std::vector<int32_t> test_data(test_index->getData(), test_index->getData() + config->testSize);
    test.swap(test_data);

    if(config->loss == "gamma"){
        filterNonZeroTarget();
    }

    std::cout << "# Train observations : " << train.size() << std::endl;
    std::cout << "# Test observations : " << test.size() << std::endl;

    random = std::uniform_int_distribution<std::mt19937::result_type>(0, train.size() - 1);
}

int Dataset::next(){
    return train[random(generator)];
}

void Dataset::filterNonZeroTarget(){
    /*
    std::vector<int> nonZeroTarget;
    for(int i = 0; i < y_data->getSize(); i++){
        if(y[i] == 0) continue;
        nonZeroTarget.push_back(i);
    }
    std::shuffle(nonZeroTarget.begin(), nonZeroTarget.end(), generator);
    std::size_t const testSize = static_cast<std::size_t>(nonZeroTarget.size() * testPct);
    train = std::vector<int>(nonZeroTarget.begin(), nonZeroTarget.end() - testSize);
    std::sort(train.begin(), train.end());
    test = std::vector<int>(nonZeroTarget.end() - testSize, nonZeroTarget.end());
    std::sort(test.begin(), test.end());
    random = std::uniform_int_distribution<std::mt19937::result_type>(0, train.size());
    */
    float* w = weight_data->getData();

    auto end_train = std::remove_if(train.begin(), train.end(), [w](int i){return w[i] == 0;});
    train.erase(end_train, train.end());

    auto end_test = std::remove_if(test.begin(), test.end(), [w](int i){return w[i] == 0;});
    test.erase(end_test, test.end());
}
