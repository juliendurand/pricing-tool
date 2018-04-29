#include <iostream>

#include "CDPoissonRegressor.h"

CDPoissonRegressor::CDPoissonRegressor(int p, int n,
    uint8_t* x, float* y, float* exposure, int nbCoeffs,
    const std::vector<int> &offsets, std::vector<std::string> &features) : ALinearRegressor(p, n, x, y, exposure, nbCoeffs, offsets, features)
{
    sy = new double[nbCoeffs + 1];
    syp = new double[nbCoeffs + 1];
    sx = new double[nbCoeffs + 1];
}

CDPoissonRegressor::~CDPoissonRegressor(){
    delete[] sy;
    delete[] syp;
    delete[] sx;
}

void CDPoissonRegressor::fit(int i, float learning_rate)
{
    uint8_t* xi = x + p * i;

    double r = learning_rate * (y[i]- pred(i));
    coeffs[0] += r;
    for(int j = 0; j < p ; j++){
        coeffs[offsets[j]+ xi[j] + 1] += r;
    }
}

void CDPoissonRegressor::blockfit(Dataset &ds, int blocksize){
    double eta = 0.000001;
    /*for(int j = 0; j < nbCoeffs + 1 ; j++){
        sy[j] = 0;
        syp[j] = 0;
        sx[j] = 0;
    }*/

    /*std::vector<int> range(blocksize);

    for(int k = 0; k < blocksize; k++){
        range[k] = ds.next();
    }*/

    //for(int i = 0; i < 1000; i++){
        for(int j = 0; j < nbCoeffs + 1 ; j++){
            int ssy = 0;
            int ssyp = 0;
            int ssx = 0;

            for(int i = 0; i < blocksize; i++){
                int k = ds.next();
                ssy += y[k];
                ssyp += pred(k);
                ssx += 1;
            }

            if(ssx < 10) continue;
            double r = (ssy + eta) / (ssyp + eta);
            coeffs[j] += std::log(r) / ssx;
        }
    //}
}
