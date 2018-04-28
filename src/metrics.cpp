#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>

#include "metrics.h"

LinearRegressionResult::LinearRegressionResult(int p, int n, uint8_t* x, float* y,
    const std::vector<float> &ypred, float* exposure, double* coeffs):
    p(p), n(n), x(x), y(y), ypred(ypred), exposure(exposure), coeffs(coeffs)
{

}

double LinearRegressionResult::rmse(const std::vector<int> &samples){
    double rmse = 0;
    double sexp = 0;
    for(int j=0; j < samples.size(); j++){
        int i = samples[j];
        double e = y[i] - ypred[i];
        rmse += e * e;
        sexp += exposure[i];
    }
    return std::sqrt(rmse/sexp);
}

double LinearRegressionResult::gini(const std::vector<int> &samples){
    double gini = area_lorentz_fast(samples);
    return gini;
}

std::vector<size_t> reverse_sort_indexes(const std::vector<float> &v, const std::vector<int> &samples) {
  // initialize original index locations
  std::vector<size_t> idx(samples.size());
  std::iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  std::sort(idx.begin(), idx.end(),
       [&v, &samples](size_t i1, size_t i2) {return v[samples[i1]] > v[samples[i2]];});

  return idx;
}

double LinearRegressionResult::area_lorentz_fast(const std::vector<int> &samples){
    std::vector<size_t> idx = reverse_sort_indexes(ypred, samples);
    double exposure_sum = 0;
    double obs_sum = 0;
    double rank_obs_sum = 0;
    for(int i : idx){
        int obs = samples[i];
        double e = exposure[obs];
        exposure_sum += e;
        obs_sum += e * y[obs];
        rank_obs_sum += y[obs] * e * (exposure_sum - 0.5 * e);
    }
    return 1 - (2 / (exposure_sum * obs_sum)) * rank_obs_sum;
}
