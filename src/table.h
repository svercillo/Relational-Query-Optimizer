#ifndef TABLE_H
#define TABLE_H

#include "primary_key.h"
#include "statistics.h"

#include <unordered_map>
#include <vector>
#include <unordered_set>

struct Statistics;

struct Table{
    public:
        std::vector<Column*> column_vals;
        std::unordered_map<std::string, Column *> columns;
        std::unordered_set<std::string> hash_index_col_names; // contains the name of a column which has a hash index
        std::unordered_set<std::string> tree_index_col_names; // contains the name of a column which has a tree index
        PrimaryKey *primary_key;
        std::string table_name;
        Statistics *statistics;

        Table(){}

        Table(std::string table_name){
            this->table_name = table_name;
        }
};

#endif // TABLE_H