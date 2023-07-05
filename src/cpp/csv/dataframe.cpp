#include "dataframe.h"


size_t hash_c_string(const char* p, size_t s){
    // FNV-1a hash function (see wikipedia for details)
    uint64_t hash = 0xcbf29ce484222325;
    const uint64_t prime = 0x00000100000001B3;
    for (size_t i = 0; i < s; ++i) {
        hash = (hash ^ p[i]) * prime;
    }
    return hash;
}

uint32_t Serie::size(){
    return data.size();
}

uint8_t Serie::set(char *str, size_t length){
    auto hash = hash_c_string(str, length);
    auto key = std::lower_bound(hkeys.begin(), hkeys.end(), hash);
    size_t pos = key - hkeys.begin();
    if((key < hkeys.end()) && (*key == hash)){
        uint8_t value = values[pos];
        data.push_back(value);
        return value;
    }
    int value = hkeys.size();
    if(value > 255){
        std::cout << "too many values for column " << name << std::endl;
        exit(1);
    }
    hkeys.insert(hkeys.begin() + pos, hash);
    values.insert(values.begin() + pos, value);
    keys.push_back(std::string(str, length));
    data.push_back(value);

    return value;
}

std::string Serie::get_key_from_value(uint8_t value){
    return keys[value];
}

void DataFrame::add_serie(std::string name){
    Serie serie(name);
    series.push_back(serie);
}
