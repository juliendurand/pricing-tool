#include <iostream>

#include "../cpp/Array.h"


int main(int argc, char** argv)
{
    //std::cout << std::endl << "Starting" << std::endl;

    int p = 156;
    int n = 4459542;

    Array<uint8_t> x_data("./dataset/test/mt.dat", p, n);

    uint8_t* x = x_data.getData();

    int result[p * p];
    float d[p];

    int r = 0;
    int c = 0;
    int p1 = 40;

    for(int i = 0; i < p1; i++){
        std::cout << i << std::endl;
        for(int j = i; j < p1; j++){
            for(int k = 0; k < n; k++){
                r = x[i * n + k];
                c = x [j * n + k];
                result[r * p + c] += 1;
            }
        }
    }

    for(int i = 0; i < p1; i++){
        for(int j = 0; j < i; j++){
            result[i * p + j] += result[j * p + i];
        }
    }
    /*
    float a = 1.0897;
    for(int i = 0; i < p; i++){
        uint8_t* x0 = x + i * n;
        for(int j = 0; j < n; j++){
            d[*x0] += 4.87;
            x0++;
        }
        std::cout << d[i] << std::endl;
    }*/
    //std::cout << std::endl << "Finished OK." << std::endl;
}
