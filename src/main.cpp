#include "relational_schema_parser.h"
#include "json_dumper.h"
#include "query_parser.h"
#include "statistics_parser.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <optional>
#include <set>


using namespace std;

std::string process_input_file(char * file_name){
    // Create a text string, which is used to output the text file
    
    // Read from the text file
    ifstream file(file_name);

    if (file.fail()){
        cout << printf("\nFile %s not found!!!\n", file_name) << endl;
        abort();
    }
    
    string line, contents;
    while (getline(file, line)){
        string linecpy = "";
        for (size_t i = 0; i < line.size(); i++)
        {
            if (line[i] == '='){
                linecpy += ' ';
                linecpy += line[i];
                linecpy += ' ';
            } else {
                linecpy += line[i];
            }
        }

        contents.append(linecpy);
        contents.append("\n");
    }

    file.close(); // close the file

    contents.append(" ");
    return contents;
}


void dump_output_file(char * file_name, std::string contents)
{   
    ofstream fw(file_name, std::ofstream::out);
    if (fw.is_open())
        fw << contents;

    fw.close();
}

int main(int argc, char ** argv) {
    char *input_file_name = argv[1];
    char * output_file_name = argv[2];
    ifstream f(input_file_name);

    std::string input_contents = process_input_file(input_file_name);


    // RelationalSchemaParser relational_schema_parser = RelationalSchemaParser(input_contents);
    // relational_schema_parser.fill_data_strucuture();
    

    // JsonDumper json_dumper = JsonDumper(relational_schema_parser.schema);
    // std::string output_contents = json_dumper.dump_contents();
    // dump_output_file(output_file_name, output_contents); // TODO: comment this out



    // StatisticsParser* statistics_parser = new StatisticsParser(input_contents);
    // statistics_parser->fill_data_structures();
    // statistics_parser->print_stats();


    QueryParser * query_parser = new QueryParser(input_contents);
    query_parser->fill_data_structures();
    query_parser->set_parent_and_child_pointers();



    // deallocation
    // relational_schema_parser.release_memory();

    return 0;
}
