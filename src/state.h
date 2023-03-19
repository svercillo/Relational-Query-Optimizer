#ifndef STATE_H
#define STATE_H

#include "table.h"

#include <unordered_map>
#include <unordered_set>
#include <iostream>

using namespace std;


struct State{
    
    int num_columns_per_tuple; // the total number of columns in for this node
    long num_tuples; // number of tuples

    // if applicable,   key = column name    and   value =  start, end of the value range
    // if a selection with = was used, then start and end values will be the same
    // can contain any column from any included table 
    unordered_map<string, pair<int, int>> column_ranges; // inclusive ranges
    unordered_set<string> column_names; // current column names, may contain conglomerate names

    unordered_map<string, Table *> tables_included; // tables include up unti this state

    string to_string(){
        return (
            "State: (num_columns_per_tuple: " 
            + std::to_string(num_columns_per_tuple) 
            + ", num_tuples: " 
            + std::to_string(num_tuples) 
            + ")");
    }

    State(){};
};

#endif // STATE_H