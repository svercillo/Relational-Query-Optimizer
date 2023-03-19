#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "schema.h"
#include "enums.h"
#include <cassert>
#include "table.h"
#include "foreign_key.h"
#include "column_types.h"
#include "statistics.h"
#include "utils.h"
#include "node.h"

#include <unordered_map>
#include <iostream>
#include <vector>
using namespace std;

struct Statistics;

class QueryParser{ // parses SQL queries and the statistics associated with tables
    public:
        
        QueryParser(std::string contents)
        {
            this->contents = contents;
            
        }

        void set_parent_and_child_pointers();
        void fill_data_structures();

        
        Node *get_root_node();
        
        unordered_map<std::string, Node *> get_query_map(){
            return this->query_map;
        };


    private:
        Node* root;
        
        std::string contents;
        unordered_map<std::string, Node *> query_map; // maps a temporary table to a data object 'Node'

        std::string toupper(std::string value){
            std::string upper = "";
            for (int x = 0; x < value.length(); x++)
                upper += std::toupper(value[x]);

            return upper;
        }
};

#endif // QUERY_PARSER_H