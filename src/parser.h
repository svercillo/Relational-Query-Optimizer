#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "statistics.h"
#include "schema.h"
#include "state.h"


#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;

class Parser{ // parser interface
    public:
        Parser(string contents){
            this->contents = contents;
            this->fill_data_structures();
        }
        virtual void fill_data_structures(){};

    private:
        string contents;
};

#endif //PARSER_H