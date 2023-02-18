#ifndef FOREIGN_KEY_H
#define FOREIGN_KEY_H

#include "key.h"
#include "table.h"

struct ForeignKey : public Key
{
    public:
        std::vector<Column*> reference_columns;
        Table * reference_table;
        Table * table;
        ForeignKey() {}
};

#endif // FOREIGN_KEY_H
