#ifndef MODELRESULT_H_
#define MODELRESULT_H_

#include <string>
#include <vector>


class ModelResult
{
public:
    std::vector<int> id;
    std::vector<double> y;
    std::vector<double> y_pred;
    std::vector<double> weights;
    std::vector<double> dp; // dot product of coefficients with observations

    ModelResult(int size, const std::string loss);
    void setObservation(int position, int id, double y, double y_pred,
        double weight, double dp);
    double logLikelihood();
    double rmse();
    double gini();
    void writeResults(std::string resultPath);

private:
    const std::string loss;
    const std::vector<size_t> reverse_sort_indexes (
        const std::vector<double>& v, const std::vector<double>& w);
};

#endif  // MODELRESULT_H_
