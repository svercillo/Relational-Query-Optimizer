
#ifndef SCHEMA_H
#define SCHEMA_H

#include "column.h"
#include "table.h"

#include <string>
#include <vector>
#include <map>


class Schema{ 
    public:
        std::map<std::string, Table> tables; // tables accesible by name
        int schema_id;
        Schema() : schema_id(1) {}
};

#endif // SCHEMA_H

