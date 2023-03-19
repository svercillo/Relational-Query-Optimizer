#ifndef STATISTICS_PARSER_H
#define STATISTICS_PARSER_H

#include "schema.h"
#include "enums.h"
#include <cassert>
#include "table.h"
#include "foreign_key.h"
#include "column_types.h"
#include "statistics.h"


#include <unordered_map>
#include <iostream>
#include <vector>
using namespace std;

struct Statistics;

class StatisticsParser{ 
    public:
        StatisticsParser(std::string contents)
        {
            this->contents = contents;
        }

        void fill_data_structures();

        void print_stats();

        std::unordered_map<std::string, Statistics* > get_statistics(){
            return this->statistics_per_table;
        };

        private:
        std::string contents;
        std::unordered_map<std::string, Statistics* > statistics_per_table;

        std::string toupper(std::string value){
            std::string upper = "";
            for (int x = 0; x < value.length(); x++)
                upper += std::toupper(value[x]);

            return upper;
        }
};

#endif //STATISTICS_PARSER_H