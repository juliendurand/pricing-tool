#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <ctype.h> // import isspace

#include "lib/nlohmann/json.hpp"
using json = nlohmann::json;

#include "csv.h"


void handle_error(char const *msg){
    std::cout << msg << std::endl;
    exit(1);
}

char * trim(char *str, size_t n, size_t *out){
    char *end = str + n;

    // Trim leading space
    while(str < end && isspace(*str)){
        str++;
    }

    // Trim trailing space
    while(str < end && isspace(*(end - 1))){
        end--;
    }

    *out = end - str;
    return str;
}

void csv::Parser::process_header(std::string header){
    bool processed = (std::find(features.begin(), features.end(), header) != features.end());
    process_column.push_back(processed);
    dataframe->add_serie(header);
}

void csv::Parser::process_cell(char *cell_start, size_t length){
    cell_start = trim(cell_start, length, &length);
    if(line == 0){
        process_header(std::string(cell_start, length));
        return;
    }
    if(process_column[column]){
        auto value = dataframe->series[column].set(cell_start, length);
        if(line % 100000 == 0){
            std::cout << line << "line :" << std::to_string(value) << std::endl;
            }
    }
    column++;
    cells++;
}

void csv::Parser::process_endline(){
    line++;
    column = 0;
    //std::cout << std::endl;
}

std::shared_ptr<DataFrame>  csv::Parser::parse(std::string const path){
    static const size_t BUFFER_SIZE = 16 * 1024;
    char buffer[BUFFER_SIZE];
    std::ifstream f(path, std::ios::in);
    char *cell_start = buffer + BUFFER_SIZE;

    while(f.good())
    {
        size_t nb_bytes = cell_start - buffer;
        f.read(buffer + BUFFER_SIZE - nb_bytes, nb_bytes);
        auto n = f.gcount();
        cell_start = buffer;
        for(unsigned int i = 0; i < BUFFER_SIZE - nb_bytes + n; i++){
            switch(buffer[i]){
                case '\n':
                    process_cell(cell_start, buffer + i - cell_start);
                    process_endline();
                    cell_start = buffer + i + 1;
                    break;
                case ',':
                    process_cell(cell_start, buffer + i - cell_start);
                    cell_start = buffer + i + 1;
                    break;
                default:;
            }
        }

        // error if no line found in the buffer
        if(cell_start == buffer){
            handle_error("line too long !");
        }

        // copy end of buffer at the begining
        if(cell_start < buffer + BUFFER_SIZE){
            std::memmove(buffer, cell_start, buffer + BUFFER_SIZE - cell_start);
        } else {
            cell_start = buffer + BUFFER_SIZE;
        }
    }
    std::cout << "nb lines" << line << std::endl;

    return dataframe;
}

int main(){
    std::ifstream f("config/config_dataset.json");
    json config = json::parse(f);
    csv::Parser parser(config.at("features"));
    auto dataframe = parser.parse(config.at("filename"));
    return 0;
}
