#include <cinttypes>
#include <iostream>
#include <cmath>


class ImplicitSGDPoissonRegressor
{

    public:

    int p;
    int n;
    uint8_t* x;
    double* y;
    double* exposure;
    double* coeffs;


    ImplicitSGDPoissonRegressor(int, int);
    ~ImplicitSGDPoissonRegressor();
    void fit(int, double);

};


ImplicitSGDPoissonRegressor::ImplicitSGDPoissonRegressor(int p, int n) :
    p(p), n(n)
{
    x = new uint8_t[n * p];
    y = new double[n];
    exposure = new double[n];
    coeffs = new double[p + 1];
}


ImplicitSGDPoissonRegressor::~ImplicitSGDPoissonRegressor()
{
    delete x;
    delete y;
    delete exposure;
    delete coeffs;
}

void ImplicitSGDPoissonRegressor::fit(int i, double learning_rate)
{
    uint8_t* xi = x + p * i;
    double yi = y[i];
    double ei = exposure[i];

    if(ei < 0.2){
        return;
    }

    yi /= ei;
    double dp = coeffs[0];
    for(int j = 0; j < p; j++){
        dp += xi[j] * coeffs[j + 1];
    }
    double y_pred = exp(dp);
    double r = learning_rate * (yi - y_pred);
    double x_sq = 1;
    for(int j = 0; j < p; j++){
        x_sq += xi[j] * xi[j];
    }
    double x1 = 0, x2 = r, x3 = 0;
    double y1 = x1 - learning_rate * (yi - y_pred * exp(x_sq * x1));
    double y2 = x2 - learning_rate * (yi - y_pred * exp(x_sq * x2));
    double y3 = 0;
    for(int j=0; (j < 30) && (std::abs(y2) > 1e-12); j++){
        x3 = (x1 * y2 - x2 * y1) / (y2 - y1);
        y3 = x3 - learning_rate * (yi - y_pred * exp(x_sq * x3));
        x1 = x2;
        y1 = y2;
        x2  = x3;
        y2 = y3;
    }

    coeffs[0] = x2;
    for(int j = 0; j < p ; j++){
        coeffs[j + 1] += x2 * xi[j];
    }
}


int main(){
    int p = 10;
    int n = 1000;
    ImplicitSGDPoissonRegressor model = ImplicitSGDPoissonRegressor(p, n);
    for(int i=0; i < 1000000000; i++){
       model.fit(5, 0.0001);
   }
}
