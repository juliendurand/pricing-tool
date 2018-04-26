#include <cmath>

#include "metrics.h"

LinearRegressionResult::LinearRegressionResult(int p, int n, uint8_t* x, float* y,
    float* ypred, float* exposure, float* coeffs):
    p(p), n(n), x(x), y(y), ypred(ypred), exposure(exposure), coeffs(coeffs)
{

}

double LinearRegressionResult::rmse(int nbSamples, std::vector<int> samples){
    double rmse = 0;
    double sexp = 0;
    for(int j=0; j < nbSamples; j++){
        int i = samples[j];
        double e = y[i] - ypred[i];
        rmse += e * e;
        sexp += exposure[i];
    }
    return std::sqrt(rmse/sexp);
}

double LinearRegressionResult::gini(int nbSamples, std::vector<int> samples){
    return 0;
}
