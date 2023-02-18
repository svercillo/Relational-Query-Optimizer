#ifndef KEY_H
#define KEY_H

#include "column.h"
#include <string>
#include <vector>

struct Key
{
    public:
        std::vector<Column *> key_columns;
};

#endif // KEY_H
