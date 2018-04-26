#include <cinttypes>
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

 #include "array.h"


void btof(void* bytes, float* floats, int size){
    memcpy(floats, bytes, 4 * size );
}


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
    void writeResults(std::string);
    void printGroupedCoeffN2();
};


ImplicitSGDPoissonRegressor::ImplicitSGDPoissonRegressor(int p, int n,
    uint8_t* x, float* y, float* exposure) :
    p(p), n(n), x(x), y(y), exposure(exposure)
{
    coeffs = new float[(p + 1) * 200];
    coeffs_star = new float[(p + 1) * 200];
}


ImplicitSGDPoissonRegressor::~ImplicitSGDPoissonRegressor()
{
    delete[] coeffs;
    delete[] coeffs_star;
}

void ImplicitSGDPoissonRegressor::fit(int i, float learning_rate)
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
        if(xi[j] > 200){
            std::cout << "ERROR" << std::endl;
        }
        dp += coeffs[(j + 1) * 200 + xi[j]];
    }
    double y_pred = exp(dp);
    double r = learning_rate * (yi - y_pred);
    double x_sq = 1 + p;  // optimization
    /*
    for(int j = 0; j < p; j++){
        std::cout << (int) xi[j] << std::endl;
        x_sq += xi[j] * xi[j];
    }
    */
    double x1 = 0, x2 = r, x3 = 0;
    double y1 = x1 - learning_rate * (yi - y_pred * exp(x_sq * x1));
    double y2 = x2 - learning_rate * (yi - y_pred * exp(x_sq * x2));
    double y3 = 0;
    for(int j=0; (j < 30) && (std::abs(y2) > 1e-12); j++){
        //std::cout << x1 << " " << x2 << " " << y1 << " " << y2 << std::endl;
        x3 = (x1 * y2 - x2 * y1) / (y2 - y1);
        y3 = x3 - learning_rate * (yi - y_pred * exp(x_sq * x3));
        x1 = x2;
        y1 = y2;
        x2 = x3;
        y2 = y3;
        //std::cout << x1 << " " << x2 << " " << y1 << " " << y2 << std::endl;
    }
    //std::cout << yi << " " << y_pred << " " << dp << " "<< x_sq << " " << r << " " << x2 << std::endl;
    coeffs[0] += x2;
    for(int j = 0; j < p ; j++){
        coeffs[(j + 1) * 200 + xi[j]] += x2;
    }
    for(int j = 0; j < (p+1) * 200 ; j++){
        coeffs_star[j] += coeffs[j];
    }
}

void ImplicitSGDPoissonRegressor::fit_explicit(int i, float learning_rate)
{
    uint8_t* xi = x + p * i;

    double yi = y[i];
    double ei = exposure[i];

    double dp = coeffs[0];
    for(int j = 0; j < p; j++){
        if(xi[j] > 200){
            std::cout << "ERROR" << std::endl;
        }
        dp += coeffs[(j + 1) * 200 + xi[j]];
    }
    double y_pred = exp(dp) * ei;
    double r = learning_rate * (yi - y_pred);

    coeffs[0] += r;
    for(int j = 0; j < p ; j++){
        coeffs[(j + 1) * 200 + xi[j]] += r;
    }
}

int ImplicitSGDPoissonRegressor::penalizeLasso(float learning_rate, float l1){
    int nb_coeffs_non_zero = 0;
    for(int j = 0; j < p * 200 ; j++){
        float c = coeffs[200 + j];
        if(c > l1 * learning_rate){
            coeffs[200 + j] -= l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else if(c < -l1 * learning_rate){
            coeffs[200 + j] += l1 * learning_rate;
            nb_coeffs_non_zero++;
        } else {
            coeffs[200 + j] = 0;
        }
    }
    return nb_coeffs_non_zero;
}

int ImplicitSGDPoissonRegressor::penalizeGroupLasso(float learning_rate, float l1){
    int nb_coeffs_non_zero = 0;
    for(int i = 0; i< p; i++){
        float s = 0;
        for(int j = 0; j < 200 ; j++){
            float c = coeffs[(i + 1) *200 + j];
            s += c * c;
        }
        s = std::sqrt(s);
        if(s > l1 * learning_rate){
            for(int j = 0; j < 200 ; j++){
                coeffs[(i + 1) *200 + j] -= l1 * learning_rate * coeffs[(i + 1) *200 + j] / s;
            }
            nb_coeffs_non_zero++;
        } else {
            for(int j = 0; j < 200 ; j++){
                coeffs[(i + 1) *200 + j] = 0;
            }
        }
    }
    return nb_coeffs_non_zero;
}

void ImplicitSGDPoissonRegressor::penalizeRidge(float learning_rate, float l2){
    for(int j = 0; j < p * 200 ; j++){
        coeffs[200 + j] *= (1 - l2 * learning_rate);
    }
}

double ImplicitSGDPoissonRegressor::pred(int i){
    uint8_t* xi = x + p * i;
    double dp = coeffs[0];
    for(int j = 0; j < p; j++){
        if(xi[j] > 200){
            std::cout << "ERROR" << std::endl;
        }
        dp += coeffs[(j + 1) * 200 + xi[j]];
    }
    return exp(dp) * exposure[i];
}

void ImplicitSGDPoissonRegressor::writeResults(std::string filename){
    std::ofstream resultFile;
    resultFile.open(filename.c_str(), std::ios::out);
    resultFile << "exposure" << "," << "target" << "," << "prediction" << std::endl;
    for(int i = 0; i < 1000000; i++){
        pred(i);
        resultFile << exposure[i] << "," << y[i] << "," << pred(i) << std::endl;
    }
    resultFile.close();
}

void ImplicitSGDPoissonRegressor::printGroupedCoeffN2(){
    for(int i = 0; i< p; i++){
        float s = 0;
        for(int j = 0; j < 200 ; j++){
            float c = coeffs[(i + 1) *200 + j];
            s += c * c;
        }
        std::cout << i << ",:" << std::sqrt(s) << "," << std::endl;
    }
    std::cout << std::endl;
}


int main(){
    int p = 152;
    int n = 4459543;

    Array<uint8_t> x("./data/observations.dat", p, n);
    Array<float> y_data("./data/targets.dat", 1, n * 4);
    Array<float> exposure_data("./data/exposure.dat", 1, n * 4);
    float* y = y_data.getData();
    float* exposure = exposure_data.getData();
    ImplicitSGDPoissonRegressor model(p, n, x.getData(), y, exposure);
    int nb_iterations = 5000000;
    double alpha = 1;
    for(int i=0; i < nb_iterations; i++){
        alpha = 3000 / float(10000 + i - 1);
        alpha = 0.0001;
        model.fit_explicit(i % 3000000, alpha);
        int nb_coeffs = 0;
        nb_coeffs = 0;
        model.penalizeRidge(alpha, 0.0001);
        if(i%100000 == 0){
            double rmse = 0;
            double s = 0;
            for(int j=3000000; j < n; j++){
                double e = y[j] - model.pred(j);
                rmse += e*e;
                s += exposure[j] ;
            }
            rmse = std::sqrt(rmse/s);
            std::cout << "rmse: (" << i << ") " << rmse << " " <<  nb_coeffs << std::endl;
       }
    }


    double rmse = 0;
    double s = 0;
    for(int i=3000000; i < n; i++){
        double e = y[i] - model.pred(i);
        rmse += e*e;
        s += exposure[i] ;
    }
    rmse = std::sqrt(rmse/s);
    std::cout << "rmse: " << rmse << std::endl;


    /*for(int j = 0; j < (p+1) * 200 ; j++){
        model.coeffs[j] = model.coeffs_star[j] / nb_iterations;
    }*/


    rmse = 0;
    s = 0;
    for(int i=3000000; i < n; i++){
        double e = y[i] - model.pred(i);
        rmse += e*e;
        s += exposure[i];
    }
    rmse = std::sqrt(rmse/s);
    std::cout << "rmse of mean coeffs: " << rmse << std::endl;

    model.printGroupedCoeffN2();

    model.writeResults("./data/results.csv");

    std::cout << "Finished OK." << std::endl;
}
