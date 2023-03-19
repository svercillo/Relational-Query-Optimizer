#ifndef COST_CALC_H
#define COST_CALC_H

#include "node.h"
#include "statistics.h"
#include "schema.h"
#include "state.h"


#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;

class QueryStateCalculator{
    public:
        Schema *schema;
        unordered_map<std::string, Statistics *> statistics_per_table;
        Node *root_node;
        unordered_map<std::string, Node *> query_map;
        int columns_per_page;

        QueryStateCalculator(
            Schema *schema_,
            unordered_map<std::string, Statistics *> statistics_per_table_,
            Node *root_node_,
            unordered_map<std::string, Node *> query_map_
        ){
            this->schema = schema_;
            this->statistics_per_table = statistics_per_table_;
            this->root_node = root_node_;
            this->query_map = query_map_;
            this->columns_per_page = get_columns_per_page();
        };

        void push_projections_above_selections();
        int sum_total_query_cost();
        vector<Node *> zero_function_node_cost();

        void calculate_query_state();

    private:
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

        int get_columns_per_page();
        Table *find_base_table_of_column(string conglomerate_col_string, State state);
        

        // ----------------- STATE CALCULATIONS -----------------
        // stack allocated result values are fine

        State calculate_state(Node *node);
        State calculate_state(BaseTable *basetable);
        State calculate_state(Selection *selection);
        State calculate_state(Projection *projection);
        State calculate_state(Join *join);
        State calculate_state_selection_range(Selection *selection);
        State calculate_state_selection_equality(Selection *selection);

        void calculate_state_selection_equality_size(Selection *selection, State *state_p, State *current_state_p);
        void calculate_state_selection_equality_cost(Selection *selection, State *current_state); // helper function
        void calculate_cost_without_optimization(Join *join, State *left_state_p, State *right_state_p);
        void calculate_cost_optimizing_left_side(Join *join, State *left_state_p, State *right_state_p);
        void calculate_cost_optimizing_right_side(Join *join, State *left_state_p, State *right_state_p);
        void calculate_state_join_size(Join *join, State *left_state_p, State *right_state_p, State *current_state);
        
};

#endif //COST_CALC_H