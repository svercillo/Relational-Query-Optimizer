#ifndef FOREIGN_KEY_H
#define FOREIGN_KEY_H

#include "key.h"

struct ForeignKey : public Key
{
    public:
        std::vector<Column> reference_columns;
        std::string reference_table_name;
        ForeignKey(){}
};

#endif // FOREIGN_KEY_H
