#include <fstream>
#include <sstream>
#include <iostream>

#include "transactions_parser.h"
#include "cascadeless_scheduler.h"
#include "recoverable_scheduler.h"


using namespace std;

std::string get_input_string(char * file_name){
    // convert input file to string
    string content;
    ifstream file(file_name);

    if (file) {
        stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        file.close();
    } else {
        cout << "Error: unable to open file." << endl;
        abort();
    }

    return content;
    
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

    std::string input_contents = get_input_string(input_file_name);

    TransactionsParser * parser = new TransactionsParser(input_contents);
    CascadelessScheduler * cascadeless_scheduler;
    // RecoverableScheduler * recoverable_scheduler;

    vector<const Action *> actions_vec = parser->actions_vec;

    cascadeless_scheduler = new CascadelessScheduler(actions_vec);
    cascadeless_scheduler->schedule_tasks();

    // recoverable_scheduler = new RecoverableScheduler(actions_vec);
    // recoverable_scheduler->schedule_tasks();

    std::cout << cascadeless_scheduler->to_string() << endl;
    // std::cout << recoverable_scheduler->to_string() << endl;

    delete parser;
    delete cascadeless_scheduler;
    // delete recoverable_scheduler;

    return 0;
}