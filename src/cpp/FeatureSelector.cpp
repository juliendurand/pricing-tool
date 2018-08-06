#include "FeatureSelector.h"

#include <iostream>


FeatureSelector::FeatureSelector(SGDRegressor* model):
    model(model)
{
}

void FeatureSelector::backwardStepwise(long& i){
    std::cout << "Backward Stepwise" << std::endl;
    for(;;){
        model->fitEpoch(i, 1);
        if(model->selected_features.size() > 0){
            int remove_feature = model->getMinCoeff(model->selected_features);
            model->storeFeatureInGiniPath(remove_feature);
            model->eraseFeatures({remove_feature});
        }
        else{
            model->storeFeatureInGiniPath(-1);
            break;
        }
    }
}

void FeatureSelector::forwardStepwise(long& i, int maxNbFeatures){
    std::cout << "Forward Stepwise" << std::endl;
    model->eraseAllFeatures();
    for(auto p : model->giniPath){
        int f = p.feature_idx;
        if(f >= 0){
            model->addFeatures({f});
            model->fitEpoch(i, 1);
            model->storeFeatureInGiniPath(f);
            if(model->selected_features.size() >= maxNbFeatures){
                break;
            }
        }
    }
}

void FeatureSelector::fit(){
    std::cout << std::endl << "Fit Model for "
                           << model->config->nbFeaturesInModel
                           << " variables :" << std::endl;

    long i = 0;
    double stopCriterion = (model->config->loss == "poisson") ? 0.00001 : 0.000001;
    model->fitUntilConvergence(i, 1, stopCriterion);
    model->printResults();
    backwardStepwise(i);

    int maxSortedFeatures = model->config->p;
    for(int k = 0; k < 6; k++){
        std::vector<int> bestFeatures = model->getBestFeatures(maxSortedFeatures, 0.0001);
        maxSortedFeatures = bestFeatures.size();
        forwardStepwise(i, maxSortedFeatures);
    }
    maxSortedFeatures = std::min(maxSortedFeatures, model->config->nbFeaturesInModel);
    std::vector<int> bestFeatures = model->getBestFeatures(maxSortedFeatures, 0.0002);
    forwardStepwise(i, maxSortedFeatures);
    model->eraseAllFeatures();
    model->addFeatures(bestFeatures);
    model->fitUntilConvergence(i,  5, stopCriterion);
}

void FeatureSelector::printSelectedFeatures(){
    std::cout << "Selected Features :" <<std::endl;
    for(int i = 1; i < model->selected_features.size() + 1; i++){
        FeatureResult& p = model->giniPath[i];
        std::cout << "        " << i << " : " << p.feature
                  << " [N2=" << p.norm
                  << ", CGini" << p.coeffGini * 100
                  << "%, Spread(100/0)=" << p.spread100 * 100
                  << "%, Spread(95/5)=" << p.spread95 * 100 << "%]"
                  << std::endl;
    }
}
