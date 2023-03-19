#ifndef KEY_H
#define KEY_H

#include "column.h"
#include <string>
#include <vector>

struct Key
{
    public:
        std::vector<Column *> key_columns;

        std::vector<std::string> get_key_column_names(){
            std::vector<std::string> key_column_names;
            for (auto col : key_columns){
                key_column_names.push_back(col->column_name);
            }

            return key_column_names;
        };
};

#endif // KEY_H
