#ifndef RELATIONAL_SCHEMA_PARSER_H
#define RELATIONAL_SCHEMA_PARSER_H

#include "schema.h"
#include "enums.h"
#include "table.h"
#include "foreign_key.h"
#include "column_types.h"
// #include "utils.h"

#include <iostream>
#include <vector>

using namespace std;

class RelationalSchemaParser{
    public:
        std::string contents;
        Schema* schema;
        RelationalSchemaParser(std::string contents) {
            this->contents = contents;
        }
        void fill_data_strucuture();
        void release_memory();

    private:
        void process_table_statemeent(
            std::vector<std::string> primary_key_col_names,
            Table *table,
            Schema *schema,
            std::vector<std::string> column_vals,
            std::vector<std::string> foreign_key_reference_table_names,
            std::vector<std::vector<std::string> > foreign_key_col_names,
            std::vector<std::vector<std::string> > foreign_key_reference_col_names
        );

        void process_foriegn_key_statement(
            std::vector<std::string> primary_key_col_names,
            Table * table,
            Schema * schema,
            std::vector<std::string> foreign_key_reference_table_names,
            std::vector<std::vector<std::string> > foreign_key_col_names,
            std::vector<std::vector<std::string> > foreign_key_reference_col_names
        );

        std::string toupper(std::string value){
            std::string upper = "";
            for (int x = 0; x < value.length(); x++)
                upper += std::toupper(value[x]);

            return upper;
        }


};

#endif // RELATIONAL_SCHEMA_PARSER_H