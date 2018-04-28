
#include <cmath>
#include <iostream>
#include <map>

#include "array.h"
#include "metrics.h"

#include "SGDPoissonRegressor.h"
#include "CDPoissonRegressor.h"
#include "ImplicitSGDPoissonRegressor.h"


void covarianceProduct(double* c, uint8_t* x,
    const std::vector<int> &samples, const std::vector<int> &offsets,
    int nbTotalModalities, int p, float* exposure)
{
    //Flush with zeros
    for(int i = 0; i < nbTotalModalities * nbTotalModalities; i++){
        c[i] = 0;
    }

    for(int i : samples){
        uint8_t* xi = x + p * i;
        double e = std::exp(exposure[i]);
        double v = e * e;
        for(int j = 0; j < p; j++){
            int pos = (offsets[j] + xi[j]) * nbTotalModalities;
            for(int k = j; k < p; k++){
                c[pos + offsets[k] + xi[k]] += v;
            }
        }
    }

    // Complete the lower triangular matrix
    for(int j = 0; j < nbTotalModalities; j++){
        for(int k = j + 1; k < nbTotalModalities; k++){
            c[k * nbTotalModalities + j] = c[j * nbTotalModalities + k];
        }
    }
}

void yProduct(double* c, uint8_t* x, const std::vector<int> &samples,
    const std::vector<int> &offsets,
    int nbTotalModalities, int p, float* y, float* exposure)
{
    //Flush with zeros
    for(int i = 0; i < nbTotalModalities; i++){
        c[i] = 0;
    }

    for(int i : samples){
        uint8_t* xi = x + p * i;
        for(int j = 0; j < p; j++){
            c[offsets[j] + xi[j]] += y[i] * exposure[i];
        }
    }
}

std::vector<std::string> features = {
"AUTV_prix_sra_max",
 "AUTV_prix_sra_min",
 "AUTV_rm",
 "AUTV_segment_max",
 "AUTV_segment_min",
 "CLI_banque",
 "POL_cdopmrh",
 "POL_fract",
 "POL_cdregion",
 "HAB_cdresid",
 "CLI_sex",
 "POL_distrib",
 "AUTV_energie_es",
 "AUTV_energie_go",
 "AUTV_k8000_n",
 "AUTV_k8000_o",
 "AUTV_prix_sra_ad",
 "AUTV_prix_sra_ef",
 "AUTV_prix_sra_gi",
 "AUTV_prix_sra_jl",
 "AUTV_prix_sra_mn",
 "AUTV_prix_sra_supo",
 "AUTV_segment_0",
 "AUTV_segment_a",
 "AUTV_segment_b",
 "AUTV_segment_h",
 "AUTV_segment_m1",
 "AUTV_segment_m2",
 "AUTV_nb_rc_12",
 "AUTV_nb_rc_34",
 "AUTV_nb_rc_56",
 "AUTV_nb_rc_78",
 "AUTV_nb_rc_sup9",
 "AUTV_nb_vol_ab",
 "AUTV_nb_vol_cd",
 "AUTV_nb_vol_ef",
 "AUTV_nb_vol_gh",
 "AUTV_nb_vol_ij",
 "AUTV_nb_vol_supk",
 "HAB_inser",
 "POL_mtcapass",
 "CLI_indice",
 "AUTA_tot1a",
 "AUTA_tot2a",
 "AUTA_tot3a",
 "AUTA_tot4a",
 "POL_tx_objv",
 "POL_tr_capass",
 "CLI_age",
 "annee",
 "CLI_nb_contrat",
 "CLI_nb_contrat_atr",
 "CLI_nb_contrat_aup",
 "CLI_nb_contrat_bqe",
 "CLI_nb_contrat_epa",
 "CLI_nb_contrat_hab",
 "CLI_nb_contrat_mot",
 "CLI_nb_contrat_mrp",
 "CLI_nb_contrat_pf",
 "CLI_nb_contrat_pju",
 "CLI_nb_contrat_prv",
 "CLI_nb_contrat_san",
 "HAB_nb_pieces",
 "CLI_nb_enfant",
 "POL_fr",
 "AUTA_b50_n",
 "AUTA_b50_o",
 "AUTA_rm_50",
 "AUTA_rm_5060",
 "AUTA_rm_6070",
 "AUTA_rm_7090",
 "AUTA_rm_inf50",
 "AUTA_rm_sup90",
 "HAB_surf_dep",
 "HAB_inhab",
 "AUTV_nb_rc",
 "AUTV_nb_vol",
 "POL_pac",
 "POL_paj",
 "POL_pan",
 "POL_pap",
 "POL_par",
 "CLI_prod_entree",
 "HAB_qual",
 "POL_ran",
 "CLI_top_atr",
 "CLI_top_aup",
 "CLI_top_bqe",
 "CLI_top_epa",
 "CLI_top_hab",
 "CLI_top_mot",
 "CLI_top_mrp",
 "CLI_top_pf",
 "CLI_top_pju",
 "CLI_top_prv",
 "CLI_top_san",
 "HAB_top_jardin",
 "CLI_tr_age",
 "CLI_tr_ancli",
 "CLI_tr_mtvi",
 "CLI_tr_pm",
 "HAB_habit_X_qual_X_inhab",
 "POL_mtobv",
 "POL_tr_tx_objv",
 "CLI_cspf",
 "REG_zoneAxa_a",
 "REG_zoneAxa_m",
 "Random",
 "HAB_anclg",
 "CLI_sit_fam2",
 "AUTA_rm",
 "HAB_hab",
 "HAB_habit",
 "POL_delec",
 "POL_vol",
 "CLI_tr_age_X_CSPf",
 "ddea_quant_freq_20",
 "ddea_quant_cm_10",
 "ddem_quant_freq_20",
 "ddem_quant_cm_20",
 "clim_quant_pp_20",
 "vol_quant_freq_20",
 "vol_quant_cm_20",
 "bdg_quant_freq_20",
 "bdg_quant_cm_20",
 "catnat_quant_pp_10",
 "fire_quant_freq_20",
 "HAB_habit_nbpieces",
 "HAB_qual_inhab",
 "POL_distrib_code",
 "Zone_inc_cm_iris",
 "Zone_inc_freq_iris",
 "zone_vol_cm_insee",
 "zone_vol_freq_insee"
};

int main(){
    int p = 134;
    int n = 4459543;
    Dataset ds(n, 0.2);

    Array<uint8_t> x_data("./data/observations.dat", p, n);
    uint8_t* x = x_data.getData();
    Array<float> y_data("./data/targets.dat", 1, n * 4);
    float* y = y_data.getData();
    Array<float> exposure_data("./data/exposure.dat", 1, n * 4);
    float* exposure = exposure_data.getData();

    std::vector<int> nbModalities = {
        7, 7, 2, 7, 7, 45, 7, 3, 6, 2, 3, 11, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 6, 16, 25, 28, 34, 6, 31, 131, 5, 51, 47, 27, 22, 34, 51, 16, 30, 13, 11, 16, 14, 15, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 7, 2, 2, 2, 2, 2, 11, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 9, 10, 10, 9, 25, 6, 12, 12, 17, 10, 4, 4, 7, 1, 2, 2, 2, 81, 21, 11, 21, 21, 21, 21, 21, 21, 21, 11, 21, 30, 4, 4, 8, 21, 16, 21
    };

    std::vector<int> offsets(nbModalities.size() + 1);
    for(int i = 1; i < nbModalities.size() + 1; i++){
        offsets[i] = offsets[i - 1] + nbModalities[i - 1];
    }
    int nbTotalModalities = offsets.back();

    std::set<int> selected_features;
    for(int i=0; i < p; i++){
        selected_features.insert(i);
    }
    selected_features.erase(111);

/*
    std::cout << "Starting CovProduct " << ds.train.size() << std::endl;
    Array<double>* covdata = new Array<double>("./data/covproduct.dat", nbTotalModalities, nbTotalModalities * 8);
    double* c = covdata->getData();
    covarianceProduct(c, x, ds.train, offsets, nbTotalModalities, p, exposure);
    //for(int i=0; i < 10; i++){
    //    std::cout << c[i] << std::endl;
    //}
    delete covdata;

    std::cout << "Starting yP " << ds.train.size() << std::endl;
    Array<double>* yp = new Array<double>("./data/yp.dat", 1, nbTotalModalities * 8);
    c = yp->getData();
    yProduct(c, x, ds.train, offsets, nbTotalModalities, p, y, exposure);
    //for(int i=0; i < 10; i++){
    //    std::cout << c[i] << std::endl;
    //}
    delete yp;
    std::cout << "Finishing" << std::endl;

    return 0;
*/

    SGDPoissonRegressor model(p, n, x, y, exposure, nbTotalModalities, offsets, features);

    int nb_iterations = 70000000;
    int nbMaxSelectedFeatures = 12;

    for(int i=0; i < nb_iterations; i++){
        double alpha = 0.0001;
        //alpha = 30000/ float(100000 + i - 1);
        //(i < 10000000) alpha = 0.01 + (0.00001 - 0.01)/ 10000000 * i;

        model.filterfit(ds.next(), alpha, selected_features);
        if(selected_features.size() <= nbMaxSelectedFeatures){
            model.penalizeRidge(alpha, 0.0001);
        }

        //int non_zero = model.penalizeLasso(alpha, 0.01);
        if(i % 10000000 == 0){
            std::vector<float> ypred = model.predict();
            LinearRegressionResult res(p, n, x, y, ypred, exposure, model.coeffs);
            std::cout << i << " : rmse train=" << res.rmse(ds.train) << ", test=" << res.rmse(ds.test) << ")" << " | gini(train=" << res.gini(ds.train) << ", test=" << res.gini(ds.test) << ")" << std::endl;
            //std::cout << non_zero << std::endl;
        }
        if(i > 10000000 &&  i % 300000 == 0 && selected_features.size() > nbMaxSelectedFeatures){
            int remove_feature = model.getMinCoeff(selected_features);
            std::cout << "Removing: " << features[remove_feature] << " Norm2=" << model.getCoeffNorm2(remove_feature) << std::endl;
            selected_features.erase(remove_feature);
            for(int j = offsets[remove_feature]; j < offsets[remove_feature + 1]; j++){
                model.coeffs[j + 1] = 0;
            }
       }
    }

    std::map<double, int> keep_features;
    for(auto f : selected_features){
        double v = model.getSpread(f);
        keep_features[v] = f;
    }

    for(auto kv = keep_features.rbegin(); kv != keep_features.rend(); kv++){
        std::cout << "Keep: " << features[kv->second] << " N2: " << model.getCoeffNorm2(kv->second) << " Spread 100/0: " << model.getSpread(kv->second) << "%" << std::endl;
    }

    std::vector<float> ypred = model.predict();
    LinearRegressionResult result(p, n, x, y, ypred, exposure, model.coeffs);
    std::cout << "rmse (train) : " << result.rmse(ds.train) << std::endl;
    std::cout << "rmse (test) : " << result.rmse(ds.test) << std::endl;
    std::cout << "gini (train) : " << result.gini(ds.train) << std::endl;
    std::cout << "gini (test) : " << result.gini(ds.test) << std::endl;
/*
    int blocksize = 10000;
    for(int i=0; i < nb_iterations / blocksize; i++){

        model.blockfit(ds, 10000, 0.5);
        if((i * blocksize ) % 10000000 == 0){
            std::vector<float> ypred = model.predict();
            LinearRegressionResult res(p, n, x, y, ypred, exposure, model.coeffs);
            std::cout << i * blocksize << " : rmse train=" << res.rmse(ds.train) << ", test=" << res.rmse(ds.test) << ")" << " | gini(train=" << res.gini(ds.train) << ", test=" << res.gini(ds.test) << ")" << std::endl;
        }
    }


    ypred = model.predict();
    result = LinearRegressionResult(p, n, x, y, ypred, exposure, model.coeffs);
    std::cout << "rmse (train) : " << result.rmse(ds.train) << std::endl;
    std::cout << "rmse (test) : " << result.rmse(ds.test) << std::endl;
    std::cout << "gini (train) : " << result.gini(ds.train) << std::endl;
    std::cout << "gini (test) : " << result.gini(ds.test) << std::endl;
*/
    //model.printGroupedCoeffN2();

    model.writeResults("./data/results.csv", ds.test);

    std::cout << "Finished OK." << std::endl;
}
