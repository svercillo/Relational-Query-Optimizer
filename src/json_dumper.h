#ifndef JSON_DUMPER_H
#define JSON_DUMPER_H

#include "column.h"
#include "schema.h"

#include <string>
#include <vector>

class JsonDumper
{
    public:
        Schema * schema;
        JsonDumper(Schema * schema)
        {
            this->schema = schema;
        }
        std::string dump_contents();

    private:
        void add_columns_to_contents(std::string *pContents, Table *pTable);
        void add_primary_key_to_contents(std::string *pContents, Table *pTable);
        void add_foriegn_keys_to_contents(std::string *pContents, std::vector<ForeignKey *> foreign_keys);
        void add_table_to_contents(std::string *pContents, Schema *schema, std::vector<Table *>::iterator it);
};

#endif // JSON_DUMPER_H