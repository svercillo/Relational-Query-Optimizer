#include "query_state_calculator.h"

#define HASH_COST 1.2 
#define NUMBER_OF_BUFFER_PAGES 3

// ------------ HELPER FUNCTIONS ------------
bool column_is_fk(vector<ForeignKey*> foreign_keys, string column_name){
    for (auto fk : foreign_keys){
        vector<string> fk_col_names = fk->get_key_column_names();
        if (column_name == fk_col_names[0]){
            return true;
        }
    }
    return false;
};

double logB(int N, int B) {
    return log(static_cast<double>(N)) / log(static_cast<double>(B));
};

bool string_contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
};

State copy_state(const State& other){

    State state;
    state.num_columns_per_tuple = other.num_columns_per_tuple;
    state.num_tuples = other.num_tuples;

    for (auto entry : other.column_ranges) {
        pair<int, int> range = entry.second;
        state.column_ranges[entry.first] = make_pair(range.first, range.second);
    }

    for (auto col_name : other.column_names){
        state.column_names.insert(col_name);
    } // deep copy column_names

    for (auto entry : other.tables_included)
    {
        state.tables_included[entry.first] = entry.second;
    }

    return state;
};
// ------------

void QueryStateCalculator::calculate_query_state(){
    this->calculate_state(this->root_node);
}

int QueryStateCalculator::get_columns_per_page(){
    int max_num_cols_in_page = 0;

    for (auto entry : this->statistics_per_table){
        string table_name = entry.first;
        Statistics *statistics = entry.second;
        Table *table = get_table_info(table_name);

        int num_cols = table->columns.size();
        double cardilaity = statistics->cardinality;
        double pages_in_table = statistics->size;

        double cols_in_table = num_cols * cardilaity;

        // this is an approximation based on the data given
        // however we know that the page size must be at least this large
        int num_cols_per_page = ceil(cols_in_table / pages_in_table);

        max_num_cols_in_page = max(max_num_cols_in_page, num_cols_per_page);
    }

    return max_num_cols_in_page;
};


int QueryStateCalculator::sum_total_query_cost(){
    unordered_map<string, Node *> query_map = this->query_map;

    int total_query_cost = 0;
    for (auto entry : query_map){
        Node *node = entry.second;
        total_query_cost += node->cost_of_operation;
        // cout << "Node " << entry.first << " cost " << total_query_cost << endl;
    }

    return total_query_cost;
};



vector<Node *> QueryStateCalculator::zero_function_node_cost(){ // sets the cost of all nodes to 0
    vector<Node *> result;

    vector<Node *> q;
    q.push_back(this->root_node);

    while (q.size() > 0){
        vector<Node *> new_q;

        for (auto node : q){
            node->cost_of_operation = 0; // set cost to zero
            node->operation_descr = "Description not set";
            State new_state;
            node->state = new_state; // set state to null

            if (node->get_type() == "selection")
            {
                Selection *selection = dynamic_cast<Selection *>(node);

                if (selection->child != nullptr)
                    new_q.push_back(selection->child);
            }
            else if (node->get_type()== "projection")
            {
                Projection *projection = dynamic_cast<Projection *>(node);

                if (projection->child != nullptr)
                    new_q.push_back(projection->child);
            }
            else if (node->get_type()== "join")
            {
                Join *join = dynamic_cast<Join *>(node);

                if (join->left_child != nullptr)
                    new_q.push_back(join->left_child);

                if (join->right_child != nullptr)
                    new_q.push_back(join->right_child);
            }
            else
            { // base table
                // pass
            }
        }
        q = new_q;
    }
    return result;
};
// ----------------- STATE CALCULATIONS -----------------

State QueryStateCalculator::calculate_state(Node * node)
{ // calculate and update a node's state, including number of columns and probability distribution after a given points

    Schema *schema = this->schema;
    string node_type = node->get_type();

    if (node_type == "table"){ // assume that base table is always the bottom of table, don't recurses
        BaseTable *basetable = dynamic_cast<BaseTable *>(node);
        return calculate_state(basetable);
    } else if (node_type == "selection"){
        Selection* selection = dynamic_cast<Selection *>(node);
        return calculate_state(selection);
    }
    else if (node->get_type() == "projection")
    {
        Projection* projection = dynamic_cast<Projection *>(node);
        return calculate_state(projection);
    } else if (node->get_type() == "join"){
        Join* join = dynamic_cast<Join *>(node);
        return calculate_state(join);
    }
    cout << "QUERY MAP" << this->query_map.size() << endl;
    State state;
    state.num_tuples = 99999999;
    return state;
    // string select_table_name = selection->input_relation;
};

State QueryStateCalculator::calculate_state(Join * join)
{
    State left_state = calculate_state(join->left_child);
    State right_state = calculate_state(join->right_child);
    State current_state;

    string conglomerate_left_join_on = join->left_join_on_col; // can contain temp table name such as A.col_name
    string act_left_join_on_col_name; // the actual column name: in A.col_name -> col_name
    if (conglomerate_left_join_on.find(".") != string::npos){
        act_left_join_on_col_name = conglomerate_left_join_on.substr(conglomerate_left_join_on.find(".") + 1);
    } else{
        act_left_join_on_col_name = conglomerate_left_join_on; // column is non-ambiguous
    }

    string conglomerate_right_join_on = join->right_join_on_col; // can contain temp table name such as A.col_name
    string act_right_join_on_col_name; // the actual column name: in A.col_name -> col_name
    if (conglomerate_right_join_on.find(".") != string::npos){
        act_right_join_on_col_name = conglomerate_right_join_on.substr(conglomerate_right_join_on.find(".") + 1);
    } else{
        act_right_join_on_col_name = conglomerate_right_join_on; // column is non-ambiguous
    }
    // --------- COST CALCULATION ---------
    join->cost_of_operation = INT_MAX;

    calculate_cost_optimizing_left_side(join, &left_state, &right_state);
    calculate_cost_optimizing_right_side(join, &left_state, &right_state);

    calculate_cost_without_optimization(join, &left_state, &right_state);

    // --------- SIZE CALCULATION ---------
    calculate_state_join_size(join, &left_state, &right_state, &current_state);

    join->state = current_state;

    return current_state;
};

State QueryStateCalculator::calculate_state(Projection *projection)
{
    State state = calculate_state(projection->child); // assume that selection can never be bottom of table

    
    State current_state = copy_state(state); // performs a DEEP copy via State constructor


    for (auto entry : state.tables_included)
    {
        string table_name = entry.first;
        Table *table = get_table_info(table_name);

        for (auto column_entry : table->columns){
            string actual_col_name = column_entry.first; // actual col_names
            bool table_col_in_select;


            // check if col in projection

            bool in_projection = false;
            string conglom;
            for (auto conglomerate_col_string : projection->column_names)
            { // conglomerate_col_string may contain "OP1.column_name" instead of "column_names"
                if (string_contains(conglomerate_col_string, actual_col_name)){
                    in_projection = true;
                    break;
                }
            }

            if (!in_projection){
                // congolmerate is stored in state not actual column name
                current_state.column_ranges[actual_col_name] = make_pair(INT_MIN, INT_MIN);
            }
        }
    }

    projection->cost_of_operation = 0;                                     // assume all projections can be done on the fly
    current_state.num_columns_per_tuple = projection->column_names.size(); // only project number of columns left

    current_state.column_names.clear(); // wipe all old column names
    for (auto col_name : projection->column_names){
        current_state.column_names.insert(col_name);
    }
    
    projection->state = current_state;



    return current_state;
};

State QueryStateCalculator::calculate_state(BaseTable *basetable)
{
    // get tuple and column statistics directly from base table
    State current_state;
    string table_name = basetable->table_name;
    Table *table = get_table_info(table_name);
    Statistics* statistics = get_table_statistics(table_name);

    int num_columns_per_tuple = table->columns.size();
    double num_tuples = statistics->cardinality;

    current_state.num_tuples = num_tuples;
    current_state.num_columns_per_tuple = num_columns_per_tuple;

    unordered_map<string, pair<int, int>> column_ranges;
    current_state.column_ranges = column_ranges;

    basetable->cost_of_operation = 0; // no cost associated with a base_table place holder node
    current_state.tables_included[table_name] = table; // append table to tables included

    for (auto entry : table->columns){
        Column *column = entry.second;
        current_state.column_names.insert(column->column_name);
    }

    basetable->state = current_state;
    
    return current_state;
};


Table * QueryStateCalculator::find_base_table_of_column(string conglomerate_col_string, State state){
    string actual_col_name; // the actual column name: in A.col_name -> col_name
    State basestate = state;
    if (conglomerate_col_string.find(".") != string::npos){
        actual_col_name = conglomerate_col_string.substr(conglomerate_col_string.find(".") + 1);
        string temp_table = conglomerate_col_string.substr(0, conglomerate_col_string.find("."));

        Node *node = this->query_map[temp_table];
        basestate = node->state;
    } else {
        actual_col_name = conglomerate_col_string; // column is non-ambiguous
    }

    for (auto entry : basestate.tables_included)
    {
        Table *table = entry.second;

        for (auto col_entry : table->columns){
            string col_name = col_entry.first;

            if (col_name == actual_col_name){
                return table;
            }
        }
    }

    cout << "ERROR CALCULATING STATE, TABLE NOT FOUND FOR COL " << conglomerate_col_string << endl;
    return nullptr;
}

State QueryStateCalculator::calculate_state(Selection *selection){ // TODO: refactor into functions
    


    
    State state = calculate_state(selection->child); // assume that selection can never be bottom of table
    State current_state = copy_state(state); // performs a DEEP copy via State constructor


    Table *table = find_base_table_of_column(selection->column_name, state);

    
    Statistics *statistics = get_table_statistics(table->table_name);
    vector<string> prim_key_col_names = table->primary_key->get_key_column_names();
    vector<ForeignKey*> foreign_keys = schema->foreign_keys[table->table_name];


    int total_num_cols = state.num_columns_per_tuple * state.num_tuples;
    int num_pages = static_cast<int>(ceil(static_cast<double>(total_num_cols) / this->columns_per_page));

    string conglomerate_col_string = selection->column_name; // can contain temp table name such as A.col_name
    string actual_col_name; // the actual column name: in A.col_name -> col_name
    if (conglomerate_col_string.find(".") != string::npos){
        actual_col_name = conglomerate_col_string.substr(conglomerate_col_string.find(".") + 1);
    } else{
        actual_col_name = conglomerate_col_string; // column is non-ambiguous
    }

    int min_cost_operation = INT_MAX;

    double match_ratio; // the ratio of tuples that will match this condition given uniform distribution
    bool column_in_state_context = false;
    int range_start;
    int range_end;

    
    

    // cout << "HERE " << selection->id << endl;
    // cout << "col " << conglomerate_col_string << endl;


    // check if column in state context
    if (state.column_ranges.find(conglomerate_col_string) != state.column_ranges.end())
    {
        pair<int, int> range = state.column_ranges[conglomerate_col_string];
        range_start = range.first;
        range_end = range.second;
        column_in_state_context = true;
    }
    else
    {
        // column has not had selection previously applied to it
        // apply range directly on DB statistics

        vector<int> range_vec;

        if (statistics->column_range.find(actual_col_name) != statistics->column_range.end())
        {
            range_vec = statistics->column_range[actual_col_name];
        } else {
            //ASSUMPTION: if range not in statistics, assume uniform distribution of range from 1->100
            range_vec.push_back(1);
            range_vec.push_back(100);

        }

        // assume DB ranges are inclusive
        range_start = range_vec[0]; 
        range_end = range_vec[1];
    }

    
    pair<int, int> range;


    if (
        (selection->comparison_value + 1 > range_end) && selection->comparison_operator == ">"
        || (
            selection->comparison_value > range_end 
            || selection->comparison_value < range_start
        ) && selection->comparison_operator == "="
    ){
        match_ratio = 0;
        range.first = INT_MIN; 
        range.second = INT_MIN; // every other range will enter this loop if end is set to -1 
    }
    else
    {
        if (selection->comparison_operator == "=")
        {
            if (!column_in_state_context){
                vector<string> col_name = {selection->column_name};
                match_ratio = statistics->column_tup_rf[col_name]; // pull match ration directly from column statistics
            } else {
                // implies that equality is on an already existing range
                match_ratio = static_cast<double>(1) / (range_end + 1 - range_start); // assume uniform distribution
            }

            range.first = selection->comparison_value; 
            range.second = selection->comparison_value;
        }
        else
        {
            match_ratio = static_cast<double>(range_end - (selection->comparison_value + 1)) / (range_end - range_start + 1);

            range.first = selection->comparison_value + 1; // update range to be right most subset, +1 b/c range is inclusive
            range.second = range_end; // range end is never updated because we onyl have '>' range operators
        }
    }
    


    current_state.column_ranges[conglomerate_col_string] = range; // set range to updated range
    
    // cout <<"\trange_start " << range.first << " " << range.second << endl;

    // calculate number of mathcing pages
    int matching_tuples = state.num_tuples * match_ratio;
    int matching_pages = (int) ceil((double) matching_tuples * state.num_columns_per_tuple/ this->columns_per_page);


    // if primary key use sorted key
    if (actual_col_name == prim_key_col_names[0]){ // we can only consider an index selection or sorted selection
        int cost_sorted_eq = ceil(logB(num_pages, 2) + matching_pages); // cost of reading from a sorted file
        
        if (cost_sorted_eq < min_cost_operation){
            min_cost_operation = cost_sorted_eq;
            selection->operation_descr = "sorted_file_eq";
        }
    }

    
    // check if search column is in hash
    if (table->hash_index_col_names.find(actual_col_name) != table->hash_index_col_names.end()){
        if (selection->comparison_operator == "="){

            // ASSUMPTION: there are no overflow buckets on simple equality selection
            // ASSUMPTION: worst case, select column is not candidate key
            // cost of hash to get index bucket, plust cost of reading a page per matching tuple in the worst case
            int cost_hash_matching = HASH_COST + matching_tuples; 
            
            if (cost_hash_matching < min_cost_operation){
                min_cost_operation = cost_hash_matching;
                selection->operation_descr = "hash_eq";
            }
        } else {
            // ASSUMPTION: index is about 10% of table size in number of pages 
            // cost of reading all pages that match, and getting each tuple in index
            int cost_range_matching = (double) 0.1 * matching_pages + matching_tuples; 

            if (cost_range_matching < min_cost_operation){
                min_cost_operation = cost_range_matching;
                selection->operation_descr = "hash_range";
            }
        }
    }
    if (table->tree_index_col_names.find(actual_col_name) != table->tree_index_col_names.end()){  // there is a FK on this column

        // get height of FK tree
        vector<string> fk_col_name = {actual_col_name};
        int tree_height = statistics->column_tup_index_height[fk_col_name];
        
        int cost_tree_eq = tree_height + matching_pages; // cost of traversing tree, plus the cost of retreiving matching pages from base table

        if (cost_tree_eq < min_cost_operation){
            min_cost_operation = cost_tree_eq;
            selection->operation_descr = "tree_eq";
        }

        // cout << "FK FOUND !!!" << endl;
        // cout << "match ratio " << match_ratio << endl;
        // cout << "matching_tuples " << matching_tuples << endl;
        // cout << "total_num_cols " << total_num_cols << endl;
        // cout << "matching pages " << matching_pages << endl;
        // cout << "TREE COST " << cost_tree_eq << endl;
    }


    // consider cost of full table scan 
    int cost_table_scan = num_pages; // number of total number of page reads

    if (cost_table_scan < min_cost_operation){
        min_cost_operation = cost_table_scan;
        selection->operation_descr = "table_scan";
    }
    
    // Note: hash index is not considered because it is not faster than a full table scan as index is stored in an undefined order and is slower
    
    selection->cost_of_operation = min_cost_operation; // set the cost if the operation
    current_state.num_tuples = matching_tuples; // update the number of tuples matching

    
    selection->state = current_state;
    return current_state;
};

void QueryStateCalculator::calculate_state_join_size(Join* join, State * left_state_p, State * right_state_p, State * current_state)
{
    State left_state = *left_state_p; // read-only 
    State right_state = *right_state_p; // read-only

    /* ASSUMPTION:

        At worst case in a join, all the tuples of one table match all the tuples of another table on a key. That 
        implies that the number of tuples in the output of the join will be less than or equal to the number of 
        tuples in either of the inupt tables. 

        This implies that for a lower bound worst case, we can keep the range distributions for our columns, because 
        that will give us a worst case estimate of cost.
    */
    int min_cost_operation = INT_MAX;
    int total_num_cols_left_child = left_state.num_columns_per_tuple * left_state.num_tuples;
    int total_num_cols_right_child = right_state.num_columns_per_tuple * right_state.num_tuples;

    string conglomerate_join_on_right_col = join->right_join_on_col; // can contain temp table name such as A.col_name
    string actual_join_on_right_col; // the actual column name: in A.col_name -> col_name
    if (conglomerate_join_on_right_col.find(".") != string::npos){
        actual_join_on_right_col = conglomerate_join_on_right_col.substr(conglomerate_join_on_right_col.find(".") + 1);
    } else{
        actual_join_on_right_col = conglomerate_join_on_right_col; // column is non-ambiguous
    }

    for (auto left_col_name : left_state.column_names){
        if (right_state.column_names.count(left_col_name)){
            // remove ambiguity for all ambiguous column names not in join criteria
            string new_left_col_name = join->left_child->id + left_col_name;
            string new_right_col_name = join->right_child->id + left_col_name;

            // add new column names
            current_state->column_names.insert(new_left_col_name);
            current_state->column_names.insert(new_right_col_name);

            // copy old pairs into new column names
            current_state->column_ranges[new_left_col_name] = make_pair(
                left_state.column_ranges[left_col_name].first,
                left_state.column_ranges[left_col_name].second);

            current_state->column_ranges[new_right_col_name] = make_pair(
                left_state.column_ranges[left_col_name].first,
                left_state.column_ranges[left_col_name].second);
        }
        else
        {
            // add new column names
            current_state->column_names.insert(left_col_name);

            // copy old pairs into new column names
            current_state->column_ranges[left_col_name] = make_pair(
                left_state.column_ranges[left_col_name].first,
                left_state.column_ranges[left_col_name].second);
        }
    }

    for (auto right_col_name : right_state.column_names){

        // we don't want to add the right join column as it's dropped from the outputs
        if (!left_state.column_names.count(right_col_name) && right_col_name != actual_join_on_right_col){
            // add new column names
            current_state->column_names.insert(right_col_name);

            // copy old pairs into new column names
            current_state->column_ranges[right_col_name] = make_pair(
                left_state.column_ranges[right_col_name].first,
                left_state.column_ranges[right_col_name].second);
        }
    }
    
    // add all the columns from both tables minus a redundant join column
    int num_columns_per_tuple = left_state.num_columns_per_tuple + right_state.num_columns_per_tuple - 1;
    
    int num_tuples = min(left_state.num_tuples, static_cast<long>(right_state.num_columns_per_tuple)); // this is the worst case upper bound

    // combine included tables from left and right nodes
    for (auto entry : left_state.tables_included) {
        current_state->tables_included[entry.first] = entry.second;
    }
    for (auto entry : right_state.tables_included) {
        current_state->tables_included[entry.first] = entry.second;
    }
    
    
    
    current_state->num_columns_per_tuple = num_columns_per_tuple;
    current_state->num_tuples = num_tuples;
}

void QueryStateCalculator::calculate_cost_without_optimization(Join* join, State * left_state_p, State * right_state_p)
{
    State left_state = *left_state_p; // read only 
    State right_state = *right_state_p; // read-only

    int min_cost_operation = join->cost_of_operation;
    int total_num_cols_left_child = left_state.num_columns_per_tuple * left_state.num_tuples;
    int total_num_cols_right_child = right_state.num_columns_per_tuple * right_state.num_tuples;

    int num_pages_left = static_cast<int>(
        ceil(static_cast<double>(total_num_cols_left_child) / this->columns_per_page));

    int num_pages_right = static_cast<int>(
        ceil(static_cast<double>(total_num_cols_right_child) / this->columns_per_page));

    int materialization_cost_right;
    if (join->right_child->get_type() == "table")
        materialization_cost_right = 0; // do not need to materialize base table
    else
        materialization_cost_right = num_pages_right; 


    int materialization_cost_left;
    if (join->left_child->get_type() == "table")
        materialization_cost_left = 0; // do not need to materialize base table
    else
        materialization_cost_left = num_pages_left;


    // calculate cost of sort merge join with materialization cost
    int cost_sort_merge_join = (
        2 * ceil(logB(num_pages_left, NUMBER_OF_BUFFER_PAGES-1))   // sort left temp table (read / write)
        + 2 * ceil(logB(num_pages_right, NUMBER_OF_BUFFER_PAGES-1)) // sort rigth temp table (read / write)
        + (num_pages_left + num_pages_right) // cost of merge
        + materialization_cost_left  + materialization_cost_right // no pipelining in 
    );


    if (cost_sort_merge_join < min_cost_operation)
    {
        min_cost_operation = cost_sort_merge_join;
        join->operation_descr = "sort_merge_join";
    }

    // calculate cost of simple nested loop with left as outter
    int cost_simple_nested_left_outter = (
        num_pages_left + num_pages_left * num_pages_right + materialization_cost_left + materialization_cost_right
    ); // cost of simple nested loop plus materialization cost

    if (cost_simple_nested_left_outter < min_cost_operation){
        min_cost_operation = cost_simple_nested_left_outter;
        join->operation_descr = "simple_nested_left_outter";
    }

    // calculate cost of simple nested loop with right as outter
    int cost_simple_nested_right_outter = (
        num_pages_right + num_pages_right * num_pages_left + materialization_cost_left + materialization_cost_right
    ); // cost of simple nested loop plus materialization cost

    if (cost_simple_nested_right_outter < min_cost_operation){
        min_cost_operation = cost_simple_nested_right_outter;
        join->operation_descr = "simple_nested_right_outter";
    }

    

    if (min_cost_operation < join->cost_of_operation){
        join->cost_of_operation = min_cost_operation;
    }
};

void QueryStateCalculator::calculate_cost_optimizing_left_side(Join* join, State * left_state_p, State * right_state_p)
{
    State left_state = *left_state_p; // read only
    State right_state = *right_state_p; // read-only

    string conglomerate_left_join_on = join->left_join_on_col; // can contain temp table name such as A.col_name
    string act_left_join_on_col_name; // the actual column name: in A.col_name -> col_name
    if (conglomerate_left_join_on.find(".") != string::npos){
        act_left_join_on_col_name = conglomerate_left_join_on.substr(conglomerate_left_join_on.find(".") + 1);
    } else{
        act_left_join_on_col_name = conglomerate_left_join_on; // column is non-ambiguous
    }
    
    int min_cost_operation = join->cost_of_operation;
    int total_num_cols_left_child = left_state.num_columns_per_tuple * left_state.num_tuples;
    int total_num_cols_right_child = right_state.num_columns_per_tuple * right_state.num_tuples;

    int num_pages_left = static_cast<int>(
        ceil(static_cast<double>(total_num_cols_left_child) / this->columns_per_page));

    int num_pages_right = static_cast<int>(
        ceil(static_cast<double>(total_num_cols_right_child) / this->columns_per_page));

    int materialization_cost_right;
    if (join->right_child->get_type() == "table")
        materialization_cost_right = 0; // do not need to materialize base table
    else
        materialization_cost_right = num_pages_right; 


    int materialization_cost_left;
    if (join->left_child->get_type() == "table")
        materialization_cost_left = 0; // do not need to materialize base table
    else
        materialization_cost_left = num_pages_left; 

    unordered_map<string, Table *> left_included_tables;
    if (conglomerate_left_join_on != act_left_join_on_col_name)
    {
        // get node id
        string node_id = conglomerate_left_join_on.substr(0, conglomerate_left_join_on.find("."));
        left_included_tables = this->query_map[node_id]->state.tables_included; // set included tables to node_id included tables
    } else { 
        // included tables is just from left_child state
        left_included_tables = left_state.tables_included;
    }

    bool join_on_left_primary_key = false;

    // check if pipelining in index nested loop join is possible
    for (auto entry : left_included_tables){ // see if we can do INL pipelined join on left table

        Table *table = entry.second;
        string table_name = table->table_name;
        vector<ForeignKey *> foreign_keys = schema->foreign_keys[table_name];

        Statistics *statistics = get_table_statistics(table_name);


        if (table->primary_key->key_columns[0]->column_name == act_left_join_on_col_name){ // check if join is on file sort col
            join_on_left_primary_key = true;
        }
        if (table->hash_index_col_names.find(act_left_join_on_col_name) != table->hash_index_col_names.end()){ // hash index on left left

            
            // cost of reading pages of left, cost of tuple * hash plus cost of materializing right
            int cost_left_inl_hash_join = (num_pages_left + left_state.num_columns_per_tuple * (HASH_COST + 1) + materialization_cost_right); 
            
            if (cost_left_inl_hash_join < min_cost_operation){
                min_cost_operation = cost_left_inl_hash_join;
                join->operation_descr = "left_inl_hash_join";
            }
        }
        if (table->tree_index_col_names.find(act_left_join_on_col_name) != table->tree_index_col_names.end()){ // tree index on left join on col
            vector<string> fk_col_name = {act_left_join_on_col_name};
            int tree_height = statistics->column_tup_index_height[fk_col_name];

            int cost_left_inl_tree_join = (
                num_pages_left + left_state.num_columns_per_tuple * (tree_height + 1)  + materialization_cost_right
            ); // cost of reading pages of left, cost of tuple * hash plus cost of materializing right

            if (cost_left_inl_tree_join < min_cost_operation){
                min_cost_operation = cost_left_inl_tree_join;
                join->operation_descr = "left_inl_tree_join";
            }
        }        
    }

    if (join_on_left_primary_key){
        // calculate cost of sort merge join with pipelined left table
        int cost_sort_merge_join_left_pipline = (
            2 * ceil(logB(num_pages_left, NUMBER_OF_BUFFER_PAGES-1))   // sort left temp table (read / write)
            + 2 * ceil(logB(num_pages_right, NUMBER_OF_BUFFER_PAGES-1)) // sort rigth temp table (read / write)
            + (num_pages_left + num_pages_right) // cost of merge
            + 0 + materialization_cost_right // pipeline left temp table
        );

        if (cost_sort_merge_join_left_pipline < min_cost_operation){
            min_cost_operation = cost_sort_merge_join_left_pipline;
            join->operation_descr = "sort_merge_join_left_pipline";
        }
    }

    if (min_cost_operation < join->cost_of_operation)
        join->cost_of_operation = min_cost_operation; // set minimum cost
};

void QueryStateCalculator::calculate_cost_optimizing_right_side(Join* join, State * left_state_p, State * right_state_p)
{
    State left_state = *left_state_p; // read only
    State right_state = *right_state_p; // read-only

    string conglomerate_right_join_on = join->right_join_on_col; // can contain temp table name such as A.col_name
    string act_right_join_on_col_name; // the actual column name: in A.col_name -> col_name
    if (conglomerate_right_join_on.find(".") != string::npos){
        act_right_join_on_col_name = conglomerate_right_join_on.substr(conglomerate_right_join_on.find(".") + 1);
    } else{
        act_right_join_on_col_name = conglomerate_right_join_on; // column is non-ambiguous
    }
    
    int min_cost_operation = join->cost_of_operation;
    int total_num_cols_left_child = left_state.num_columns_per_tuple * left_state.num_tuples;
    int total_num_cols_right_child = right_state.num_columns_per_tuple * right_state.num_tuples;

    int num_pages_left = static_cast<int>(
        ceil(static_cast<double>(total_num_cols_left_child) / this->columns_per_page));

    int num_pages_right = static_cast<int>(
        ceil(static_cast<double>(total_num_cols_right_child) / this->columns_per_page));

    int materialization_cost_right;
    if (join->right_child->get_type() == "table")
        materialization_cost_right = 0; // do not need to materialize base table
    else
        materialization_cost_right = num_pages_right; 


    int materialization_cost_left;
    if (join->left_child->get_type() == "table")
        materialization_cost_left = 0; // do not need to materialize base table
    else
        materialization_cost_left = num_pages_left; 

    unordered_map<string, Table *> right_included_tables;
    if (conglomerate_right_join_on != act_right_join_on_col_name)
    {
        // get node id
        string node_id = conglomerate_right_join_on.substr(0, conglomerate_right_join_on.find("."));
        right_included_tables = this->query_map[node_id]->state.tables_included; // set included tables to node_id included tables
    } else { 
        // included tables is just from right_child state
        right_included_tables = right_state.tables_included;
    }

    bool join_on_right_primary_key = false;

    // check if pipelining in index nested loop join is possible
    for (auto entry : right_included_tables){ // see if we can do INL pipelined join on right table

        Table *table = entry.second;
        string table_name = table->table_name;
        vector<ForeignKey *> foreign_keys = schema->foreign_keys[table_name];

        Statistics *statistics = get_table_statistics(table_name);


        if (table->primary_key->key_columns[0]->column_name == act_right_join_on_col_name){ // check if join is on file sort col
            join_on_right_primary_key = true;
        }
        if (table->hash_index_col_names.find(act_right_join_on_col_name) != table->hash_index_col_names.end()){ // hash index on right right

            
            // cost of reading pages of right, cost of tuple * hash plus cost of materializing right
            int cost_right_inl_hash_join = (num_pages_right + right_state.num_columns_per_tuple * (HASH_COST + 1) + materialization_cost_left); 
            
            if (cost_right_inl_hash_join < min_cost_operation){
                min_cost_operation = cost_right_inl_hash_join;
                join->operation_descr = "right_inl_hash_join";
            }
        }
        if (table->tree_index_col_names.find(act_right_join_on_col_name) != table->tree_index_col_names.end()){ // tree index on right join on col
            vector<string> fk_col_name = {act_right_join_on_col_name};
            int tree_height = statistics->column_tup_index_height[fk_col_name];

            int cost_right_inl_tree_join = (
                num_pages_right + right_state.num_columns_per_tuple * (tree_height + 1)  + materialization_cost_left
            ); // cost of reading pages of left, cost of tuple * hash plus cost of materializing right

            if (cost_right_inl_tree_join < min_cost_operation){
                min_cost_operation = cost_right_inl_tree_join;
                join->operation_descr = "right_inl_tree_join";
            }
        }        
    }

    if (join_on_right_primary_key){
        // calculate cost of sort merge join with pipelined right table
        int cost_sort_merge_join_right_pipline = (
            2 * ceil(logB(num_pages_right, NUMBER_OF_BUFFER_PAGES-1))   // sort right temp table (read / write)
            + 2 * ceil(logB(num_pages_right, NUMBER_OF_BUFFER_PAGES-1)) // sort rigth temp table (read / write)
            + (num_pages_right + num_pages_left) // cost of merge
            + 0 + materialization_cost_left // pipeline left temp table
        );

        if (cost_sort_merge_join_right_pipline < min_cost_operation){
            min_cost_operation = cost_sort_merge_join_right_pipline;
            join->operation_descr = "sort_merge_join_right_pipline";
        }
    }

    if (min_cost_operation < join->cost_of_operation)
        join->cost_of_operation = min_cost_operation; // set minimum cost
};
