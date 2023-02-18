
#ifndef SCHEMA_H
#define SCHEMA_H

#include "column.h"
#include "table.h"
#include "foreign_key.h"

#include <string>
#include <vector>
#include <unordered_map>


struct Schema{ 
    public:
        std::vector<Table *> table_vals;
        std::unordered_map<std::string, Table *> tables; // tables accesible by name
        std::unordered_map<std::string, std::vector<ForeignKey*> > foreign_keys;  // foreign keys
        int schema_id;
        Schema() : schema_id(1) {}
};

#endif // SCHEMA_H

