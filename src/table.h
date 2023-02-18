#ifndef TABLE_H
#define TABLE_H

#include "primary_key.h"

#include <unordered_map>
#include <vector>

struct Table{
    public:
        std::vector<Column*> column_vals;
        std::unordered_map<std::string, Column *> columns;
        PrimaryKey * primary_key;
        std::string table_name;

        Table(){}

        Table(std::string table_name){
            this->table_name = table_name;
        }
};

#endif // TABLE_H