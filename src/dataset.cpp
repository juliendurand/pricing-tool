#include "dataset.h"

Dataset::Dataset(){
}

Dataset::Dataset(Config* config, float testPct){
    std::vector<int> index(config->n);
    std::iota (std::begin(index), std::end(index), 0);
    std::random_device rd;
    generator = std::mt19937(rd());
    std::shuffle(index.begin(), index.end(), generator);
    std::size_t const testSize = static_cast<std::size_t>(config->n * testPct);
    train = std::vector<int>(index.begin(), index.end() - testSize);
    std::sort(train.begin(), train.end());
    test = std::vector<int>(index.end() - testSize, index.end());
    std::sort(test.begin(), test.end());
    random = std::uniform_int_distribution<std::mt19937::result_type>(0, train.size());

    x_data = new Array<uint8_t>(config->getFeatureFilename(), config->p, config->n);
    exposure_data = new Array<float>(config->getExposureFilename(), 1, config->n * 4);
    y_data = new Array<float>(config->getTargetFilename(), 1, config->n * 4);
}

int Dataset::next(){
    return train[random(generator)];
}
