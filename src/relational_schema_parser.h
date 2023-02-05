#ifndef RELATIONAL_SCHEMA_PARSER_H
#define RELATIONAL_SCHEMA_PARSER_H

#include "schema.h"
#include "word_types.h"
#include "table.h"

#include <iostream>


class RelationalSchemaParser{
    public:
        std::string contents;
        RelationalSchemaParser(std::string contents) {
            this->contents = contents;
        }
        Schema fill_data_strucuture();
};

#endif // RELATIONAL_SCHEMA_PARSER_H