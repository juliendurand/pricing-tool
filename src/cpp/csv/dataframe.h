#include <iostream>
#include <string>
#include <vector>

class Serie{
public:
    std::string name;
    std::vector<std::string> keys;
    std::vector<uint64_t> hkeys;
    std::vector<uint8_t> values;
    std::vector<uint8_t> data;


    Serie(std::string name) : name(name) {};
    uint32_t size();
    uint8_t set(char *str, size_t length);
    std::string get_key_from_value(uint8_t value);
};

class DataFrame{
public:
    std::vector<Serie> series;

    DataFrame() {};
    void add_serie(std::string name);
};
