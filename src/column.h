#ifndef COLUMN_H
#define COLUMN_H

#include <string>

struct Column
{
    public:
        std::string type_name;
        std::string column_name;
        std::string column_type;
        bool is_nullable;
        Column(std::string column_name){
            this->is_nullable = true;
            this->column_name = column_name;
            this->column_type = "unknown";
        }
        Column(){}
};

#endif // COLUMN_H