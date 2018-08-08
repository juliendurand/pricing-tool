#ifndef MODELRESULT_H_
#define MODELRESULT_H_

#include <string>
#include <vector>

#include "Config.h"

// Encapsulate the prediction for a glm model and provides useful metrics to
// evaluate the quality of fit.
//
// Params :
//      - size : number of observations ;
//      - config : glm regression configuration.
//
// Usage : the public methods provide the key metrics to evalate a glm fit.

class ModelResult
{
public:
    ModelResult(int size, Config* config);
    void setObservation(int position, int id, double y, double y_pred,
        double weight, double dp);
    double logLikelihood();
    double rmse();
    double gini();
    void saveResults();

private:
    Config* config;
    std::vector<int> id;
    std::vector<double> y;
    std::vector<double> y_pred;
    std::vector<double> weights;
    std::vector<double> dp; // dot product of coefficients with observations

    const std::vector<size_t> reverse_sort_indexes (
        const std::vector<double>& v, const std::vector<double>& w);
};

#endif  // MODELRESULT_H_
