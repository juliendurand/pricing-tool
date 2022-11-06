#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dataframe.h"

namespace csv{

    class Parser{
        uintmax_t line = 0;
        uintmax_t column = 0;
        uintmax_t cells = 0;
        std::vector<std::string> features;
        std::shared_ptr<DataFrame> dataframe = std::make_shared<DataFrame>();
        std::vector<std::string> headers;
        //std::unordered_map<uint64_t, uint8_t> values;
        std::vector<bool> process_column;

    public:
        std::shared_ptr<DataFrame> parse(std::string const path);

        Parser(std::vector<std::string> features) : features(features) {};

    private:

        void process_header(std::string header);
        void process_cell(char* cell_start, size_t length);
        void process_endline();
    };

}
