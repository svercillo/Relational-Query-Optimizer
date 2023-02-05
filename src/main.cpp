#include "relational_schema_parser.h"
#include "json_dumper.h"

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
        cout << "\nFile not found!!!\n" << endl;
        abort();
    }
    
    string line, contents;
    while (getline(file, line))
        contents.append(line);

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
    RelationalSchemaParser rsp = RelationalSchemaParser(input_contents);
    Schema schema = rsp.fill_data_strucuture();

    JsonDumper json_dumper = JsonDumper(schema);

    std::string output_contents = json_dumper.dump_contents();

    cout << output_contents << endl;
    dump_output_file(output_file_name, output_contents);
    return 0;
}
