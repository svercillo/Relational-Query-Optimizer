#include "tree_visualizer.h"
#define NODE_BLOCK_CHAR_LEN  15




// ----------------- HELPER FUNCTIONS -----------------
int calculate_node_frame_offset(Node *node){
    int diff = NODE_BLOCK_CHAR_LEN - node->id.size();

    if (diff > 0)
        return diff / 2;

    return 0;
}

int calculate_tree_depth(Node * root)
{ // get depth of tree
    int depth = 0;
    vector<Node *> q;
    q.push_back(root);

    while (q.size() > 0){
        vector<Node *> new_q;

        for (auto node : q){
            string node_type = node->get_type();
            if (node_type == "selection")
            {
                Selection *selection = dynamic_cast<Selection *>(node);
                new_q.push_back(selection->child);                    
            }
            else if (node_type == "projection")
            {
                Projection *projection = dynamic_cast<Projection *>(node);
                new_q.push_back(projection->child);
            }
            else if (node_type == "join")
            {
                Join *join = dynamic_cast<Join *>(node);
                new_q.push_back(join->left_child);
                new_q.push_back(join->right_child);
                
            }
        }
        depth += 1; 
        q = new_q;
    }
    return depth;
};

string convert_operation_descr_to_string(string operation_descr){

    if (operation_descr == "hash_eq"){
        return "Hash equality was performed to match this selection. Index page was retrieved, and a page per matching tuple was retrieved.";
    } else if (operation_descr == "sorted_file_eq"){
        return "Sorted file equality search was performed to match this selection on the primary key.";
    } else if (operation_descr == "tree_eq"){
        return "Tree equality was performed to match this selection.";
    } else if (operation_descr == "table_scan"){
        return "A full table scan was performed to match this selection.";
    } else if (operation_descr == "sort_merge_join"){
        return "Simple sort merge join was performed with both tables materialized if not base table.";
    } else if (operation_descr == "simple_nested_left_outter"){
        return "Simple nested join with the left table as the outter table was performed, inputs were materialized if necessary.";
    } else if (operation_descr == "simple_nested_right_outter"){
        return "Simple nested join with the right table as the outter table was performed, inputs were materialized if necessary.";
    } else if (operation_descr == "left_inl_hash_join"){
        return "Hash index nested join with the left table pipelined was performed, as join occurs on indexed column.";
    } else if (operation_descr == "left_inl_tree_join"){
        return "Tree index nested join with the left table pipelined was performed, as join occurs on indexed column.";
    } else if (operation_descr == "sort_merge_join_left_pipline"){
        return "Sort merge join occured with the left table pipelined because it is already sorted on the join column.";
    } else if (operation_descr == "simple_nested_right_outter"){
        return "Simple nested join with the right table as the outter table was performed, inputs were materialized if necessary.";
    } else if (operation_descr == "simple_nested_right_outter"){
        return "Simple nested join with the right table as the outter table was performed, inputs were materialized if necessary.";
    } else if (operation_descr == "right_inl_hash_join"){
        return "Hash index nested join with the right table pipelined was performed, as join occurs on indexed column.";
    } else if (operation_descr == "right_inl_tree_join"){
        return "Tree index nested join with the right table pipelined was performed, as join occurs on indexed column.";
    } else if (operation_descr == "sort_merge_join_right_pipline"){
        return "Sort merge join occured with the right table pipelined because it is already sorted on the join column.";
    } else {
        return "Projection operation done on the fly.";
    }
}


vector<Node*> topologically_sort_query(Node * root){

    vector<Node*> result;
    vector<Node *> q;
    q.push_back(root);

    while (q.size() > 0){
        vector<Node *> new_q;

        for (auto node : q){
            result.push_back(node);
            string node_type = node->get_type();
            if (node_type == "selection")
            {
                Selection *selection = dynamic_cast<Selection *>(node);
                new_q.push_back(selection->child);                    
            }
            else if (node_type == "projection")
            {
                Projection *projection = dynamic_cast<Projection *>(node);
                new_q.push_back(projection->child);
            }
            else if (node_type == "join")
            {
                Join *join = dynamic_cast<Join *>(node);
                new_q.push_back(join->left_child);
                new_q.push_back(join->right_child);
                
            }
        }
        q = new_q;
    }
    return result;
}
// ___________________


string TreeVisualizer::visualize_cost_str(){

    string res = "";

    vector<Node*> sorted_node_list = topologically_sort_query(this->root_node);
    for (auto entry : sorted_node_list)
    {
        Node *node = (Node *)entry;

        int num_output_pages = (int)ceil((double)node->state.num_columns_per_tuple * node->state.num_tuples / this->num_columns_per_page);
        if (node->get_type() == "table"){
            res += "Base Table ->" + node->id + "\n";
            res += "\tNumber of Pages: " + to_string(num_output_pages) + "\n";
        }
        else
        {
            res += "Operation ->" +  node->id + ": \n";
            res += "\tType: " + node->get_type() + "\n";
            res += "\tOperation Description: " + convert_operation_descr_to_string(node->operation_descr) + "\n";
            res += "\tCost of Operation (in I/Os): " + to_string(node->cost_of_operation) + "\n";
            res += "\tNumber of Output Pages: " + to_string(num_output_pages) + "\n";
        }
    }
    return res;
}

vector<vector<char>> create_output_frame(int tree_depth){
    int frame_width = 1000; // this is arbitrarily big
    vector<vector<char>> frame;
    for (int i = 0; i <= tree_depth*2; i++)
    {
        vector<char> row;
        for (int j = 0; j < frame_width; j++){
            row.push_back(' ');
        }
        frame.push_back(row);
    }
    return frame;
};



vector<vector<char>> TreeVisualizer::generate_output_frame(){
    string output = "";
    Node *root = this->root_node;
    int tree_depth = calculate_tree_depth(root);

    vector<vector<char>> frame = create_output_frame(tree_depth);

    draw_node_and_set_coords(root, &frame, 0, 50, 0);
    draw_lines_between_nodes(&frame);

    return frame;
};

void TreeVisualizer::draw_lines_between_nodes(vector<vector<char>> * frame_p){

    for (auto entry : (*query_map)){
        string node_id = entry.first;
        Node *node = entry.second;

        
        if (node_id_to_coords.find(node) == node_id_to_coords.end()){
            cout << "ERROR, CANT FIND NODE " + node_id +  node->id + " COORDS IN FRAME" << endl;
            abort();
        }

        pair<int, int> coords = node_id_to_coords[node];
        int y = coords.first;
        int x = coords.second;
        
        if (node->get_type() == "selection"){
            (*frame_p)[y + 1][x + (int)NODE_BLOCK_CHAR_LEN / 2] = '|'; // add pipe
        } else if (node->get_type() == "projection"){
            (*frame_p)[y + 1][x + (int)NODE_BLOCK_CHAR_LEN / 2] = '|'; // add pipe
        } else if (node->get_type() == "join"){
            Join *join = dynamic_cast<Join *>(node);
            pair<int, int> left_coords = node_id_to_coords[join->left_child];
            pair<int, int> right_coords = node_id_to_coords[join->right_child];

            int i;
            for (i = x + calculate_node_frame_offset(node) -1; i >= left_coords.second + NODE_BLOCK_CHAR_LEN / 2; i--)
            {
                (*frame_p)[y][i] = '-';
            }
            (*frame_p)[y+1][i] = '|';

            for (
                i = x + node->id.size() + calculate_node_frame_offset(node) +2; 
                i < right_coords.second + NODE_BLOCK_CHAR_LEN / 2;
                i++
            ){
                (*frame_p)[y][i] = '-';
            }

            (*frame_p)[y+1][i] = '|';
        }
    }
}


int TreeVisualizer::draw_node_and_set_coords(Node *node, vector<vector<char>> * frame_p, int local_depth, int width, int max_name_width_in_verticle)
{ // returns with of last char

    if (node->get_type() == "join")
    {
        Join *join = dynamic_cast<Join *>(node);

        int left_width = draw_node_and_set_coords(
            join->left_child, 
            frame_p, 
            local_depth + 1, 
            width, max_name_width_in_verticle
        ) +  NODE_BLOCK_CHAR_LEN; // start width + name size
        
        
        for (int i = 0; i < node->id.size(); i++){
            (*frame_p)[local_depth * 2][left_width + 1 + i + calculate_node_frame_offset(node)] = node->id[i];
        }
        node_id_to_coords[node] = make_pair(local_depth * 2, left_width);

        int iterations = 1;
        Node *temp = node->parent;
        while (temp != nullptr && temp->get_type() != "join")
        {
            for (int i = 0; i < temp->id.size(); i++)
                (*frame_p)[(local_depth - iterations) * 2 ][left_width + 1 + i + calculate_node_frame_offset(temp)] = temp->id[i];

            node_id_to_coords[temp] = make_pair((local_depth - iterations) * 2, left_width + 1);

            iterations++;
            temp = temp->parent;
        }
        

        // max_name_width_in_verticle = max((int) node->id.size() - 1, max_name_width_in_verticle);
        int right_width = draw_node_and_set_coords(
            join->right_child,
            frame_p,
            local_depth + 1,
            left_width + 1 + NODE_BLOCK_CHAR_LEN + 1,
            max_name_width_in_verticle);
            
        
        return right_width;
    } else {
        if (node->get_type() == "selection"){
            Selection *selection = dynamic_cast<Selection *>(node);
            
            return draw_node_and_set_coords(selection->child, frame_p, local_depth + 1, width, max_name_width_in_verticle);
        } else if (node->get_type() == "projection"){
            Projection *projection = dynamic_cast<Projection *>(node);

            return draw_node_and_set_coords(projection->child, frame_p, local_depth + 1, width, max_name_width_in_verticle);
        } else {
            int iterations = 1;
            Node *temp = node->parent;
            while (temp != nullptr && temp->get_type() != "join")
            {
                for (int i = 0; i < temp->id.size(); i++)
                    (*frame_p)[(local_depth - iterations) * 2 ][width +i + calculate_node_frame_offset(temp)] = temp->id[i];

                node_id_to_coords[temp] = make_pair((local_depth- iterations) * 2, width);
                iterations++;
                temp = temp->parent;
            }

            for (int i = 0; i < node->id.size(); i++){
                (*frame_p)[local_depth * 2][width + i + calculate_node_frame_offset(node)] = node->id[i];
            }

            node_id_to_coords[node] = make_pair(local_depth * 2, width);
            return width;
        }
    }
    return 0;
}

void print_vector_of_vectors(const std::vector<std::vector<char>>& v) {
    for (const auto& row : v) {
        for (const auto& c : row) {
            std::cout << c;
        }
        std::cout << endl;
    }
}

string TreeVisualizer::visualize_tree(){

    vector<vector<char>> frame = generate_output_frame();

    // find the width of the farthest left element
    int line_legnth = INT_MAX;
    for (auto line : frame)
        line_legnth = min(static_cast<size_t>(line_legnth), line.size());


    int furthest_left = INT_MAX;
    int furthest_right = -1;
    for (int i = 0; i < line_legnth; i++)
    {
        for (auto line : frame){
            if (line[i] != ' '){

                furthest_left = min(furthest_right, i);
                furthest_right = max(furthest_right, i);
            }
        }
    }

    string res = "";

    for (auto line :frame){
        for (int i = 0; i<=furthest_right; i++){
            res += line[i];
        }
        res += "\n";
    }

    return res;
}

