#ifndef IMPLICITSGDPOISSONREGRESSOR_H_
#define IMPLICITSGDPOISSONREGRESSOR_H_

#include <string>


class ImplicitSGDPoissonRegressor
{
public:
    int p;
    int n;
    uint8_t* x;
    float* y;
    float* exposure;
    float* coeffs;
    float* coeffs_star;


    ImplicitSGDPoissonRegressor(int, int, uint8_t*, float*, float*);
    ~ImplicitSGDPoissonRegressor();
    void fit(int, float);
    void fit_explicit(int i, float learning_rate);
    int penalizeLasso(float learning_rate, float l1);
    int penalizeGroupLasso(float learning_rate, float l1);
    void penalizeRidge(float learning_rate, float l2);
    double pred(int);
    float* predict();
    void writeResults(std::string);
    void printGroupedCoeffN2();
};

#endif  // IMPLICITSGDPOISSONREGRESSOR_H_
