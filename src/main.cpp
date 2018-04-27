
#include <cmath>
#include <iostream>
#include <vector>

#include "array.h"
#include "metrics.h"

#include "SGDPoissonRegressor.h"
#include "ImplicitSGDPoissonRegressor.h"


int main(){
    int p = 152;
    int n = 4459543;
    Dataset ds(n, 0.2);
    Dataset ds2(n, 0.1);


    Array<uint8_t> x_data("./data/observations.dat", p, n);
    uint8_t* x = x_data.getData();
    Array<float> y_data("./data/targets.dat", 1, n * 4);
    float* y = y_data.getData();
    Array<float> exposure_data("./data/exposure.dat", 1, n * 4);
    float* exposure = exposure_data.getData();

    SGDPoissonRegressor model(p, n, x, y, exposure);

    std::cout << "START " << ds2.test.size() << std::endl;
    std::vector<double> s = model.covarianceProduct(ds2.test);
    std::cout << "DONE " << ds2.test.size() << std::endl;
    /*float tot_s = 0;
    for(int i=0; i<200; i++){
        for(int j=0; j<200; j++){
            if(s[i*200 + j] > 0){
                std::cout << i << "," << j << " : " << s[i*200 + j] << std::endl;
                tot_s += s[i*200 + j];
            }
        }
    }

    std::cout << "TOTAL = " << tot_s << std::endl;
    */
    return 0;


    int nb_iterations = 10000000;
    double alpha = 1;
    for(int i=0; i < nb_iterations; i++){
        alpha = 3000 / float(10000 + i - 1);
        alpha = 0.0001;
        model.fit(ds.next(), alpha);
        if(i%1000000 == 0){
            std::vector<float> ypred = model.predict();
            LinearRegressionResult res(p, n, x, y, ypred, exposure, model.coeffs);
            std::cout << i << " : rmse train=" << res.rmse(ds.train) << ", test=" << res.rmse(ds.test) << ")" << " | gini(train=" << res.gini(ds.train) << ", test=" << res.gini(ds.test) << ")" << std::endl;
       }
    }


    std::vector<float> ypred = model.predict();
    LinearRegressionResult result(p, n, x, y, ypred, exposure, model.coeffs);
    std::cout << "rmse (train) : " << result.rmse(ds.train) << std::endl;
    std::cout << "rmse (test) : " << result.rmse(ds.test) << std::endl;
    std::cout << "gini (train) : " << result.gini(ds.train) << std::endl;
    std::cout << "gini (test) : " << result.gini(ds.test) << std::endl;

    //model.printGroupedCoeffN2();

    model.writeResults("./data/results.csv", ds.test);

    std::cout << "Finished OK." << std::endl;
}
