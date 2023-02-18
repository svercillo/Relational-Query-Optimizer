#ifndef RELATIONAL_SCHEMA_PARSER_H
#define RELATIONAL_SCHEMA_PARSER_H

#include "schema.h"
#include "word_types.h"
#include "table.h"
#include "foreign_key.h"
#include "column_types.h"

#include <iostream>
#include <vector>


class RelationalSchemaParser{
    public:
        std::string contents;
        Schema* schema;
        RelationalSchemaParser(std::string contents) {
            this->contents = contents;
        }
        void fill_data_strucuture();
        ~RelationalSchemaParser();
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

        void release_memory();
};

#endif // RELATIONAL_SCHEMA_PARSER_H