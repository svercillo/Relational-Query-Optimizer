#include "json_dumper.h"

#include <iostream>

std::string JsonDumper::dump_contents(){
    Schema schema = this->schema;
    std::string contents = "{\n\t\"schema_id\" : " + std::to_string(schema.schema_id) + "\n";
    contents += "\t\"tables\" : [ ";

    std::map<std::string, Table>::iterator it;
    int num_tabs = 1;
    for (it = schema.tables.begin(); it!= schema.tables.end(); ++it)
    {
        if (it != schema.tables.begin())
            contents += ",\n";
        std::string table_name = it->first;
        Table table = it->second;

        
        
        contents += "\n\t\t\"table_name\": \"" + table_name  + "\"";
        contents += "\n\t\t\"columns\": [";

        for (int i = 0; i < table.columns.size(); ++i)
        {
            Column column = table.columns[i];
            if (i != 0){
                contents += ",";
            }

            
            std::string nullable_str = "false";
            if (column.is_nullable)
                nullable_str = "true";


            contents += "\n\t\t\t{";
            contents += "\n\t\t\t\t\"column_name\" : \"" + column.column_name + "\",";            
            contents += "\n\t\t\t\t\"not_null\" : \"" + nullable_str +"\",";
            contents += "\n\t\t\t\t\"column_type\" : \"" + column.column_type +"\"";
            contents += "\n\t\t\t}";
        }
    }

    return contents;
}