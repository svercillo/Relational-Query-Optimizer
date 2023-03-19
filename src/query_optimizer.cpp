
#include "query_optimizer.h"


bool is_node_composite(Node * node){
    // returns if a node features to a join

    if (node == nullptr) return false;
    Node *temp = node;
    while (temp->get_type() != "table"){
        if(temp->get_type() == "join")
            return true;

        else if (temp->get_type() == "selection"){
            Selection *selection = dynamic_cast<Selection *>(temp);
            temp = selection->child;
        } else if (temp->get_type() == "projection"){
            Projection * projection = dynamic_cast<Projection *>(temp);
            temp = projection->child;
        }
    }

    return false;
}

void QueryOptimizer::optimize_query(){
    push_selections();
    generate_optimal_indexes();
};

// string get_actual_col_name(string conglomerate_col_string){
//     string actual_col_name;
//     if (conglomerate_col_string.find(".") != string::npos){
//         actual_col_name = conglomerate_col_string.substr(conglomerate_col_string.find(".") + 1);
//     } else {
//         actual_col_name = conglomerate_col_string;
//     }

//     return actual_col_name;
// }

Table * QueryOptimizer::find_base_table_of_column(string conglomerate_col_string, State state){

    string actual_col_name; // the actual column name: in A.col_name -> col_name
    State basestate = state;
    if (conglomerate_col_string.find(".") != string::npos){
        actual_col_name = conglomerate_col_string.substr(conglomerate_col_string.find(".") + 1);
        string temp_table = conglomerate_col_string.substr(0, conglomerate_col_string.find("."));

        Node *node = (*query_map)[temp_table];
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

    cout << "ERROR OPTIMIZING, TABLE NOT FOUND FOR COL " << conglomerate_col_string << endl;
    return nullptr;
}

void QueryOptimizer::generate_optimal_indexes(){
    for (auto entry : this->schema->tables){ // iterate over all tables and add every index
        Table *table = entry.second;

        for (auto col_entry : table->columns){
            Column *column = col_entry.second;

            string column_name = column->column_name;
            table->hash_index_col_names.insert(column_name); // add hash index
            table->tree_index_col_names.insert(column_name); // add tree index
        }
    }
};

vector<Node *> QueryOptimizer::get_nodes_of_type(string type){ // returns all the nodes of a type in top down order
    vector<Node *> result;

    vector<Node *> q;
    q.push_back(this->root_node);

    while (q.size() > 0){
        vector<Node *> new_q;

        for (auto node : q){
            string node_type = node->get_type();
            if (node_type == "selection")
            {

                Selection *selection = dynamic_cast<Selection *>(node);
                if (selection->child != nullptr)
                    new_q.push_back(selection->child);

                if (node_type == type)
                    result.push_back(node);
            }
            else if (node_type == "projection")
            {
                Projection *projection = dynamic_cast<Projection *>(node);

                if (projection->child != nullptr)
                    new_q.push_back(projection->child);
                
                if (node_type == type)
                    result.push_back(node);
            }
            else if (node_type == "join")
            {

                Join *join = dynamic_cast<Join *>(node);

                if (join->left_child != nullptr)
                    new_q.push_back(join->left_child);

                if (join->right_child != nullptr)
                    new_q.push_back(join->right_child);

                if (node_type == type)
                    result.push_back(node);
            }
            else
            { // base table

                if (node_type == type)
                    result.push_back(node);
            }
        }
        q = new_q;
    }
    return result;
};

void QueryOptimizer::push_selections(){

    // cout << "PUSHING SELECTIONS" << endl;
    vector<Node *> selections = get_nodes_of_type("selection");    
    reverse(selections.begin(), selections.end()); // want to get the bottom ones first


    while (selections.size() > 0){
        Selection *selection = dynamic_cast<Selection*>(selections[selections.size()-1]);
        selections.pop_back();

        while (is_node_composite(selection))
        {   
            Node *child_node = (Node *)selection->child;
            // cout << child_node->id << endl;
            if (child_node->get_type() == "projection")
            {
                Node *selection_parent_node = selection->parent;
                Projection *selection_child = dynamic_cast<Projection *>(child_node);
                Node *selection_grandchild = selection_child->child;

                // set selection's parent child
                // i.e. selection_parent_node->child = selection->child;
                if (selection_parent_node->get_type() == "selection")
                {
                    Selection * selection_parent = dynamic_cast<Selection *>(selection_parent_node);
                    selection_parent->child = selection_child;
                } else if (selection_parent_node->get_type() == "projection"){
                    Projection *selection_parent = dynamic_cast<Projection *>(selection_parent_node);
                    selection_parent->child = selection_child;
                } else if (selection_parent_node->get_type() == "join"){
                    Join * selection_parent = dynamic_cast<Join *>(selection_parent_node);
                    if (selection_parent->left_child == selection_parent){ // if selection was left childs
                        selection_parent->left_child = selection_child;
                    }
                    else
                    { // selection was right child
                        selection_parent->right_child = selection_child;
                    }
                }
                
                selection_child->parent = selection_parent_node;

                selection_child->child = selection;
                selection->parent = selection_child;

                selection->child = selection_grandchild;
                selection_grandchild->parent = selection;

            }
            else if (child_node->get_type() == "selection")
            {
                Node *selection_parent_node = selection->parent;
                Selection *selection_child = dynamic_cast<Selection *>(child_node);
                Node *selection_grandchild = selection_child->child;

                // set selection's parent child
                // i.e. selection_parent_node->child = selection->child;
                if (selection_parent_node->get_type() == "selection")
                {
                    Selection * selection_parent = dynamic_cast<Selection *>(selection_parent_node);
                    selection_parent->child = selection_child;
                } else if (selection_parent_node->get_type() == "projection"){
                    Projection *selection_parent = dynamic_cast<Projection *>(selection_parent_node);
                    selection_parent->child = selection_child;
                } else if (selection_parent_node->get_type() == "join"){
                    Join * selection_parent = dynamic_cast<Join *>(selection_parent_node);
                    if (selection_parent->left_child == selection_parent){ // if selection was left childs
                        selection_parent->left_child = selection_child;
                    }
                    else
                    { // selection was right child
                        selection_parent->right_child = selection_child;
                    }
                }
                
                selection_child->parent = selection_parent_node;

                selection_child->child = selection;
                selection->parent = selection_child;

                selection->child = selection_grandchild;
                selection_grandchild->parent = selection;
                // child_node = selection;
            }
            else if (child_node->get_type() == "join")
            {
                Table *table = find_base_table_of_column(selection->column_name, selection->state); // figure out which column select belongs to
                string selection_table_name = table->table_name;
                Node *selection_parent_node = selection->parent;

                Join *join = dynamic_cast<Join *>(child_node);
                Node *left_child = join->left_child;
                Node *right_child = join->right_child;

                if (left_child->state.tables_included.find(selection_table_name) != left_child->state.tables_included.end()){
                    Selection *left_selection = new Selection(selection, left_child->id, true);
                    left_selection->parent = join;
                    left_selection->child = left_child;
                    left_child->parent = left_selection;
                    join->left_child = left_selection;
                    join->left_table_name = left_selection->id;
                    left_selection->input_relation = left_child->id;


                    left_selection->state = selection->state;

                    selections.push_back(left_selection);

                    (*query_map)[left_selection->id] = (Node*) left_selection;
                    (*query_map).erase(selection->id);                    
                }

                if (right_child->state.tables_included.find(selection_table_name) != right_child->state.tables_included.end()){
                    Selection *right_selection = new Selection(selection, right_child->id, true);
                    right_selection->parent = join;
                    right_selection->child = right_child;
                    right_child->parent = right_selection;
                    join->right_child = right_selection;
                    join->right_table_name = right_selection->id;
                    right_selection->input_relation = right_child->id;


                    right_selection->state = selection->state;
                    selections.push_back(right_selection);

                    (*query_map)[right_selection->id] = (Node*) right_selection;
                    (*query_map).erase(selection->id);        
                }
                join->parent = selection_parent_node; // set join parent

                // set selection's parent child
                if (selection_parent_node->get_type() == "selection")
                {
                    Selection * selection_parent = dynamic_cast<Selection *>(selection_parent_node);
                    selection_parent->child = join;
                } else if (selection_parent_node->get_type() == "projection"){
                    Projection *selection_parent = dynamic_cast<Projection *>(selection_parent_node);
                    selection_parent->child = join;
                } else if (selection_parent_node->get_type() == "join"){
                    Join * selection_parent = dynamic_cast<Join *>(selection_parent_node);
                    if (selection_parent->left_child == selection_parent){ // if selection was left childs
                        selection_parent->left_child = join;
                    } else { // selection was right child
                        selection_parent->right_child = join;
                    }
                }

                break;
            } else {
                break; // base table
            }
        }
    }


    
    // cout << "PRINTING NODES "  << endl;
    // for (auto entry : (*query_map)){
    //     // cout << entry.second->to_string() << endl;

    //     if (entry.first == "PUSHED_OP4_LEFT"){

    //         Selection *projection = dynamic_cast<Selection *>(entry.second);
    //         cout << "\n\nH " + projection->id + "  CHILD " << projection->child->id << " " << projection->parent->id << endl;
    //         cout << entry.second->to_string() << endl;
    //         // Join *join = dynamic_cast<Join *>(entry.second);
    //         // cout << "\n\nJOINNNN " << endl;
    //         // cout << join->left_child->id << endl;
    //         // cout << join->right_child->id << endl;
    //         abort();
    //     }
    // }
    // abort();
}

void QueryOptimizer::push_projections_above_selections()
{
    vector<Node *> all_projections = get_nodes_of_type("projection");

    for (auto proj_node : all_projections){
        Projection * proj = dynamic_cast<Projection *>(proj_node);
        while (proj->parent != nullptr && proj->parent->get_type() != "selection"){
            // swap proj and its parent
            Selection *proj_parent = dynamic_cast<Selection *>(proj->parent);

            proj_parent->child = proj->child;   // swap pointers
            proj->parent = proj_parent->parent; // swap pointers

            proj_parent->parent = proj;
            proj->child = proj_parent;
            
        }
    }
};




void swap_children_nodes(Join * join){
    string left_join_on_col = join->left_join_on_col;
    string right_join_on_col = join->right_join_on_col;
    string left_table_name = join->left_table_name;
    string right_table_name = join->right_table_name;
    Node * left_child = join->left_child;
    Node *right_child = join->right_child;

    join->left_join_on_col = right_join_on_col;
    join->right_join_on_col = left_join_on_col;

    join->left_table_name = right_table_name;
    join->right_table_name = left_table_name;

    join->left_child = right_child;
    join->right_child = left_child;

    // parent of chilrent stay the same
}


Node * get_leftmost_join(Node * node){ // gauranteed that there is one join
    if (node->get_type() == "selection"){
        Selection *selection = dynamic_cast<Selection *>(node);
        get_leftmost_join(selection->child);
    } else if (node->get_type() == "projection"){
        Projection *projection = dynamic_cast<Projection *>(node);
        get_leftmost_join(projection->child);
    }
    
    Join * join = dynamic_cast<Join *>(node); // can't be base table
    
    if (!is_node_composite(join->left_child)){
        return (Node*) join; // this is left most join
    }
    
    return get_leftmost_join(join->left_child); // recurse on left child 

}

Node* convert_to_left_deep(Node * node){ // returns top node
    // if (!is_node_composite(node))
    //     return nullptr; // all work is done

    // if (node->get_type() != "join"){
    //     if (node->get_type() == "selection"){
    //         Selection *selection = dynamic_cast<Selection *>(node);
    //         convert_to_left_deep(selection->child);
    //     } else {
    //         Projection *projection = dynamic_cast<Projection *>(node);
    //         convert_to_left_deep(projection->child);
    //     }
    // }

    Join *join = dynamic_cast<Join *>(node);
    
    // // left node is gauranteed to be left deep already
    // // check right node is left deep
    // if (!is_node_composite(join->right_child)){
    //     return; // all work is done
    // }


    // get left most join on right side
    Join * left_most_join = dynamic_cast<Join*>(get_leftmost_join(join->right_child));

    // left_most_join->



    convert_to_left_deep(join->right_child);
}

void QueryOptimizer::convert_to_left_deep_tree(){
    Node *root = this->root_node;
    vector<Node*> join_nodes = get_nodes_of_type("join");

    vector<Node *> left_non_comp_nodes;
    vector<Node *> right_non_comp_nodes;

    
    

    // for (auto node : join_nodes)
    // {
    //     Join * join = dynamic_cast<Join*>(node);
    //     if (!is_node_composite(join->left_child)){
    //         left_non_comp_nodes.push_back(join->left_child);
    //     }
    //     if (!is_node_composite(join->right_child)){
    //         right_non_comp_nodes.push_back(join->right_child);
    //     }
    // }

    // Node *min_cost_node;
    // int min_cost = INT_MAX;
    // bool min_is_right = false;

    // for (auto node : left_non_comp_nodes){
    //     int num_pages = (double)node->state.num_tuples * node->state.num_columns_per_tuple / this->columns_per_page;
    //     if (num_pages < min_cost){
    //         min_cost = num_pages;
    //         min_is_right = false;
    //         min_cost_node = node;
    //     }
    // }
    
    // for (auto node : right_non_comp_nodes){
    //     int num_pages = (double)node->state.num_tuples * node->state.num_columns_per_tuple / this->columns_per_page;
    //     if (num_pages < min_cost){
    //         min_cost = num_pages;
    //         min_is_right = true;
    //         min_cost_node = node;
    //     }
    // }


    
};
