#include "relational_schema_parser.h"
#include "json_dumper.h"
#include "query_parser.h"
#include "statistics_parser.h"
#include "query_state_calculator.h"
#include "tree_visualizer.h"
#include "query_optimizer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
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

void print_cost_of_tree(
    QueryStateCalculator *state_calculator, 
    unordered_map<std::string, Node *> query_map_, 
    Node *root_node_,
    bool optimized
){
    int total_query_cost = state_calculator->sum_total_query_cost();

    string optimized_str;
  
    if (optimized)
        optimized_str = "optimized";
    else
        optimized_str = "unoptimized";

    cout << "\n\nTotal Query Cost (" << optimized_str << "): " << total_query_cost << " I/Os" << endl << endl;

    TreeVisualizer *visualizer = new TreeVisualizer(root_node_, &query_map_, state_calculator->columns_per_page);
    string cost_calcualtion_string = visualizer->visualize_cost_str();
    
    cout << "Cost Breakdown: " << endl;
    cout << cost_calcualtion_string << endl;



    cout << "Query Tree Visualization: " << endl << endl;
    string tree_visualization = visualizer->visualize_tree();
    cout << tree_visualization << endl;
};

int main(int argc, char ** argv) {
    char *input_file_name = argv[1];
    char * output_file_name = argv[2];
    ifstream f(input_file_name);

    std::string input_contents = process_input_file(input_file_name);

    // TODO: make all parsers implement Parser interface
    RelationalSchemaParser * relational_schema_parser =  new RelationalSchemaParser(input_contents);
    relational_schema_parser->fill_data_strucuture();
    Schema *schema_ = relational_schema_parser->get_schema();



    StatisticsParser* statistics_parser = new StatisticsParser(input_contents);
    statistics_parser->fill_data_structures();
    // statistics_parser->print_stats();
    unordered_map<std::string, Statistics *> statistics_per_table_ = statistics_parser->get_statistics();

    QueryParser * query_parser = new QueryParser(input_contents);
    query_parser->fill_data_structures();
    query_parser->set_parent_and_child_pointers();
    unordered_map<std::string, Node *> query_map_ = query_parser->get_query_map();
    Node *root_node_ = query_parser->get_root_node();


    QueryStateCalculator *state_calculator = new QueryStateCalculator(
        schema_, 
        statistics_per_table_, 
        root_node_,
        query_map_
    );
    state_calculator->calculate_query_state();
    print_cost_of_tree(state_calculator, query_map_, root_node_, false);

    QueryOptimizer *optimizer = new QueryOptimizer(
        schema_,
        statistics_per_table_,
        root_node_,
        &query_map_,
        state_calculator->columns_per_page
    );

    cout << "\n\nOPTMIZING QUERY .... " << endl << endl;
    optimizer->optimize_query();


    state_calculator->zero_function_node_cost(); // zero query state 
    state_calculator->calculate_query_state();
    print_cost_of_tree(state_calculator, query_map_, root_node_, true);



    
    // deallocation
    // relational_schema_parser.release_memory();

    return 0;
}

// JsonDumper json_dumper = JsonDumper(relational_schema_parser->schema);
// std::string output_contents = json_dumper.dump_contents();
// dump_output_file(output_file_name, output_contents); // TODO: comment this out
