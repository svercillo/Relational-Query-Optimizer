#include "json_dumper.h"

#include <iostream>


void JsonDumper::add_columns_to_contents(std::string * pContents, Table * pTable){
    (*pContents) += "\n\t\t\t\"table_name\": \"" + pTable->table_name  + "\",";
    (*pContents) += "\n\t\t\t\"columns\": [";
    for (auto it = pTable->column_vals.begin(); it!= pTable->column_vals.end(); ++it)
    {
        if (it != pTable->column_vals.begin())
            (*pContents) += ",";

        Column *column = *it;

        std::string nullable_str = "false";
        if (column->is_nullable)
            nullable_str = "true";

        std::string column_type = "";
        switch (column->column_type)
        {
            case COLUMNTYPES_INT:
            {
                column_type = "INT";
                break;
            }
            case COLUMNTYPES_FLOAT:
            {
                column_type = "FLOAT";
                break;
            }
            case COLUMNTYPES_VARCHAR:
            {
                column_type = "VARCHAR";
                break;
            }
            case COLUMNTYPES_BYTE:
            {
                column_type = "BYTE";
                break;
            }
            case COLUMNTYPES_TEXT:
            {
                column_type = "TEXT";
                break;
            }
            case COLUMNTYPES_UNKNOWN: 
            {
                column_type = "UNKNOWN";
                break;
            }
        }


        (*pContents) += "\n\t\t\t\t{";
        (*pContents) += "\n\t\t\t\t\t\"column_name\" : \"" + column->column_name + "\",";            
        (*pContents) += "\n\t\t\t\t\t\"not_null\" : \"" + nullable_str +"\",";
        (*pContents) += "\n\t\t\t\t\t\"column_type\" : \"" + column_type + "\"";
        (*pContents) += "\n\t\t\t\t}";
    }

    (*pContents) += "\n\t\t\t],";
}

void JsonDumper::add_primary_key_to_contents(std::string * pContents, Table * pTable){
    std::string primary_key_present = "false";
    if (pTable->primary_key->is_present)
        primary_key_present = "true";

    (*pContents) += "\n\t\t\t\"primary_key\" : {";
    (*pContents) += "\n\t\t\t\t\"present\" : \"" + primary_key_present + "\"";

    if (pTable->primary_key->is_present){       
        (*pContents) += ",\n\t\t\t\t\"column_names\" : [";

        for (int i =0; i < pTable->primary_key->key_columns.size(); i++){
            if (i != 0)
                (*pContents) += ",";
            
            (*pContents) += "\n\t\t\t\t\t\""+ pTable->primary_key->key_columns[i]->column_name + "\"";
        }
        (*pContents) += "\n\t\t\t\t]";
    }
    (*pContents) += "\n\t\t\t},";
}

void JsonDumper::add_foriegn_keys_to_contents(std::string * pContents, std::vector<ForeignKey*> foreign_keys){
    (*pContents) += "\n\t\t\t\"foriegn_keys\" : [";
    
    std::vector<ForeignKey*>::iterator it = foreign_keys.begin();

    for (; it < foreign_keys.end(); it++){
        if (it != foreign_keys.begin())
            (*pContents) += ",";

        ForeignKey *pFk = *it;
        (*pContents) += "\n\t\t\t\t{";

        (*pContents) += "\n\t\t\t\t\t\"key_column_names\" : [";

        for (int i =0; i < pFk->key_columns.size(); i++){
            if (i != 0)
                (*pContents) += ",";
            
            (*pContents) += "\n\t\t\t\t\t\t\"" + pFk->key_columns[i]->column_name + "\"";
        }
        (*pContents) += "\n\t\t\t\t\t],";
        

        (*pContents) += "\n\t\t\t\t\t\"reference_table\" : \"" + pFk->reference_table->table_name + "\",";
        (*pContents) += "\n\t\t\t\t\t\"reference_column_names\" : [";
        for (int i =0; i < pFk->reference_columns.size(); i++){
            if (i != 0)
                (*pContents) += ",";
            
            (*pContents) += "\n\t\t\t\t\t\t\"" + pFk->reference_columns[i]->column_name + "\"";
        }
        (*pContents) += "\n\t\t\t\t\t]";

        (*pContents) += "\n\t\t\t\t}";
    }


    (*pContents) += "\n\t\t\t]";
}


void JsonDumper::add_table_to_contents(std::string * pContents, Schema * schema, std::vector<Table*>::iterator it){

    if (it != schema->table_vals.begin())
        (*pContents) += ",";

    (*pContents) += "\n\t\t{";
    
    Table *table = *it;

    add_columns_to_contents(pContents, table);
    add_primary_key_to_contents(pContents, table);

    std::string table_name = table->table_name;

    if (schema->foreign_keys.find(table_name) != schema->foreign_keys.end())
        add_foriegn_keys_to_contents(pContents, schema->foreign_keys.find(table_name)->second);
    else
        add_foriegn_keys_to_contents(pContents, std::vector<ForeignKey*>());

    (*pContents) += "\n\t\t}";

}


std::string JsonDumper::dump_contents(){
    Schema* schema = this->schema;
    std::string contents = "{\n\t\"schema_id\" : " + std::to_string(schema->schema_id) + ",\n";
    contents += "\t\"tables\" : [ ";
    

    std::vector<Table*>::iterator it;
    for (it = schema->table_vals.begin(); it!= schema->table_vals.end(); ++it)
    {
        add_table_to_contents(&contents, schema, it);
    }
    
    contents += "\n\t]";
    contents += "\n}";

    return contents;
}