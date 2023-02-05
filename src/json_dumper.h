#ifndef JSON_DUMPER_H
#define JSON_DUMPER_H

#include "column.h"
#include "schema.h"

#include <string>
#include <vector>

class JsonDumper
{
    public:
        Schema schema;
        JsonDumper(Schema schema)
        {
            this->schema = schema;
        }
        std::string dump_contents();
};

#endif // JSON_DUMPER_H