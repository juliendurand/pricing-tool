#include "FeatureSelector.h"

#include <fstream>
#include <iostream>


FeatureSelector::FeatureSelector(SGDRegressor* model):
    model(model)
{
    giniPath = std::vector<FeatureResult>(model->getSelectedFeatures().size() + 1,
                                          FeatureResult());
}

void FeatureSelector::fit(){
    std::cout << std::endl << "Fit Model for "
                           << model->config->nbFeaturesInModel
                           << " variables :" << std::endl;

    long i = 0;
    double stopCriterion = (model->config->loss == "poisson")
                           ? 0.00001 : 0.000001;
    model->fitUntilConvergence(i, 1, stopCriterion);
    model->printResults();
    backwardStepwise(i);

    int maxSortedFeatures = model->config->p;
    for(int k = 0; k < 6; k++){
        std::vector<int> bestFeatures = getBestFeatures(maxSortedFeatures,
                                                        0.0001);
        maxSortedFeatures = bestFeatures.size();
        forwardStepwise(i, maxSortedFeatures);
    }
    maxSortedFeatures = std::min(maxSortedFeatures,
                                 model->config->nbFeaturesInModel);
    std::vector<int> bestFeatures = getBestFeatures(maxSortedFeatures, 0.0002);
    forwardStepwise(i, maxSortedFeatures);
    model->eraseAllFeatures();
    model->addFeatures(bestFeatures);
    model->fitUntilConvergence(i,  5, stopCriterion);
}

void FeatureSelector::printSelectedFeatures(){
    std::cout << "Selected Features :" <<std::endl;
    for(int i = 1; i < model->getSelectedFeatures().size() + 1; i++){
        FeatureResult& p = giniPath[i];
        std::cout << "        " << i << " : " << p.feature
                  << " [N2=" << p.norm
                  << ", CGini=" << p.coeffGini
                  << "%, Spread(100/0)=" << p.spread100
                  << "%, Spread(95/5)=" << p.spread95 << "%]"
                  << std::endl;
    }
}

void FeatureSelector::writeResults(){
    auto coeffs = model->getCoeffs();
    std::ofstream selectedFeatureFile;
    selectedFeatureFile.open(model->config->resultPath + "features.csv",
                             std::ios::out);
    selectedFeatureFile << "Feature,Gini,Spread 95/5,Spread 100/0"
                        << std::endl;
    for(int i = 0; i < model->getSelectedFeatures().size() + 1; i++){
        FeatureResult& fr = giniPath[i];
        int f = fr.feature_idx;
        selectedFeatureFile << fr.feature << ','
                            << fr.gini << ','
                            << coeffs->getSpread95(f) << ','
                            << coeffs->getSpread100(f)
                            << std::endl;
    }
    selectedFeatureFile.close();

    std::ofstream giniPathFile;
    giniPathFile.open(model->config->resultPath + "ginipath.csv",
                      std::ios::out);
    giniPathFile << "Feature,Gini,CoeffGini,Norm,Spread 100/0,Spread 95/5,RMSE"
                 << std::endl;
    for (FeatureResult& p : giniPath) {
        giniPathFile << p.feature << ","
                     << p.gini << ","
                     << p.coeffGini << ","
                     << p.norm << ","
                     << p.spread100 << ","
                     << p.spread95 << ","
                     << p.rmse
                     << std::endl;
    }
    giniPathFile.close();
}

void FeatureSelector::storeFeatureInGiniPath(int f){
    int position = model->getSelectedFeatures().size();
    FeatureResult fr;
    auto coeffs = model->getCoeffs();
    auto testResult =  coeffs->predict(model->dataset, model->dataset->getTest());
    if(position == 0){
        fr = {
            -1,
            "Intercept",
            testResult->gini(),
            0,
            0,
            0,
            0,
            testResult->rmse(),
            0
        };
        std::cout << "Storing Intercept." << std::endl;
    } else {
        fr = {
            f,
            model->config->features[f],
            testResult->gini(),
            coeffs->getCoeffGini(f),
            coeffs->getCoeffNorm2(f),
            coeffs->getSpread100(f),
            coeffs->getSpread95(f),
            testResult->rmse(),
            0
        };
        std::cout << "Storing[" << position << "] "
                  << fr.feature
                  << " Gini=" << fr.gini
                  << " Norm2=" << fr.norm
                  << " CGini=" << fr.coeffGini
                  << " Spread100=" << fr.spread100
                  << " Spread95=" << fr.spread95
                  << std::endl;
    }
    giniPath[position] = fr;
}

void FeatureSelector::backwardStepwise(long& i){
    std::cout << "Backward Stepwise" << std::endl;
    for(;;){
        model->fitEpoch(i, 1);
        if(model->getSelectedFeatures().size() > 0){
            auto coeffs = model->getCoeffs();
            int remove_feature = coeffs->getMinCoeff();
            storeFeatureInGiniPath(remove_feature);
            model->eraseFeatures({remove_feature});
        }
        else{
            storeFeatureInGiniPath(-1);
            break;
        }
    }
}

void FeatureSelector::forwardStepwise(long& i, int maxNbFeatures){
    std::cout << "Forward Stepwise" << std::endl;
    model->eraseAllFeatures();
    for(auto p : giniPath){
        int f = p.feature_idx;
        if(f >= 0){
            model->addFeatures({f});
            model->fitEpoch(i, 1);
            storeFeatureInGiniPath(f);
            if(model->getSelectedFeatures().size() >= maxNbFeatures){
                break;
            }
        }
    }
}

void FeatureSelector::sortFeatures(){
    sortFeatures(giniPath.size());
}

void FeatureSelector::sortFeatures(int maxNbFeatures){
    for(int i = 1; i < giniPath.size(); i++){
        giniPath[i].diffGini = giniPath[i].gini - giniPath[i - 1].gini;
    }
    std::sort(giniPath.begin() + 1, std::min(giniPath.end(),
              giniPath.begin() + maxNbFeatures+ 1),
        [](FeatureResult& i, FeatureResult& j) {
            return i.diffGini > j.diffGini;
        }
    );
}

const std::vector<int> FeatureSelector::getBestFeatures(int maxNbFeatures,
                                                   double treshold){
    sortFeatures(maxNbFeatures);
    std::vector<int> bestFeatures;
    for (auto p = giniPath.begin() + 1; p != giniPath.end(); p++) {
        if(p->diffGini > treshold){
            bestFeatures.push_back(p->feature_idx);
        } else {
            break;
        }
        if(bestFeatures.size() >= maxNbFeatures){
            std::cout << "Stop adding features : max nb features reached("
                      << maxNbFeatures << (").") << std::endl;
            break;
        }
    }
    return bestFeatures;
}
