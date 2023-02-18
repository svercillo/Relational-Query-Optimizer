#ifndef COLUMN_H
#define COLUMN_H

#include "column_types.h"
#include <string>

struct Column
{
    public:
        std::string column_name;
        ColumnTypes column_type;
        bool is_nullable;
        Column(std::string column_name){
            this->is_nullable = true;
            this->column_name = column_name;
            this->column_type = COLUMNTYPES_UNKNOWN;
        }
        Column(std::string column_name, ColumnTypes column_type){
            this->is_nullable = true;
            this->column_name = column_name;
            this->column_type = column_type;
        }
        Column(){}
};

#endif // COLUMN_H