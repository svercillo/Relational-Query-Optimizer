#ifndef TABLE_H
#define TABLE_H

#include "foreign_key.h"
#include "primary_key.h"

#include <vector>

struct Table{
    public:
        std::vector<Column> columns;
        PrimaryKey primary_key;
        std::vector<ForeignKey> foreign_keys;
        std::string table_name;

        Table(){}

        Table(std::string table_name){
            this->table_name = table_name;
        }
};

#endif // TABLE_H