#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "array.h"
#include "ImplicitSGDPoissonRegressor.h"
#include "metrics.h"


int main(){
    int p = 152;
    int n = 4459543;

    Array<uint8_t> x_data("./data/observations.dat", p, n);
    Array<float> y_data("./data/targets.dat", 1, n * 4);
    Array<float> exposure_data("./data/exposure.dat", 1, n * 4);
    uint8_t* x = x_data.getData();
    float* y = y_data.getData();
    float* exposure = exposure_data.getData();
    ImplicitSGDPoissonRegressor model(p, n, x, y, exposure);
    int nb_iterations = 200000;
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


    int startsample = 3000000;


    int ns = n - startsample;
    std::vector<int> samples(ns);

    for(int i = 0; i < ns; i++){
        samples[i] = i;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(samples.begin(), samples.end(), g);
    for(int i=0; i < 100; i++){
            std::cout << "shuffling " << samples[i] << std::endl;
    }

    std::vector<float> ypred = model.predict();

    LinearRegressionResult result(p, n, x, y, ypred, exposure, model.coeffs);
    std::cout << "rmse: " << result.rmse(samples) << std::endl;

    model.printGroupedCoeffN2();

    model.writeResults("./data/results.csv");

    std::cout << "Finished OK." << std::endl;
    std::cout << samples.size() << std::endl;
}
