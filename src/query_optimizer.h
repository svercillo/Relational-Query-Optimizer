#ifndef QUERY_OPTIMIZER_H
#define QUERY_OPTIMIZER_H

#include "node.h"
#include "statistics.h"
#include "schema.h"
#include "state.h"


#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;

class QueryOptimizer{
    public:
        Schema *schema;
        unordered_map<std::string, Statistics *> statistics_per_table;
        Node *root_node;
        unordered_map<std::string, Node *> * query_map;
        int columns_per_page;

        QueryOptimizer(
            Schema *schema_,
            unordered_map<std::string, Statistics *> statistics_per_table_,
            Node *root_node_,
            unordered_map<std::string, Node *> *  query_map_,
            int columns_per_page_
        ){
            this->schema = schema_;
            this->statistics_per_table = statistics_per_table_;
            this->root_node = root_node_;
            this->query_map = query_map_;
            this->columns_per_page = columns_per_page_;
        };

        void optimize_query();

    private:
        Table *find_base_table_of_column(string conglomerate_col_string, State state);
        void generate_optimal_indexes();
        void push_projections_above_selections();
        void push_selections();

        Statistics *get_table_statistics(string table_name)
        {
            if (this->statistics_per_table.find(table_name) == this->statistics_per_table.end())
            {
                cout << "INVALID ENTRY: MISSING TABLE "
                     << " FROM STATISTICS" << endl;
            }
            return this->statistics_per_table[table_name]; 
        }
        
        Table * get_table_info(string table_name){ // logical schema
            if (this->schema->tables.find(table_name) == this->schema->tables.end()){
                cout << "INVALID ENTRY: MISSING TABLE "
                     << " FROM STATISTICS" << endl;
            }
            return this->schema->tables[table_name]; 
        }

        vector<Node *> get_nodes_of_type(string type);
        vector<Node *> zero_function_node_cost(string type); // sets the cost of all nodes to 0

        void convert_to_left_deep_tree();  // assumes that selection and projections are already pushed
};

#endif // QUERY_OPTIMIZER_H