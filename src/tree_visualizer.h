#ifndef TREE_VISUALIZER_H
#define TREE_VISUALIZER_H

#include "node.h"
#include "statistics.h"
#include "schema.h"
#include "state.h"


#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;

class TreeVisualizer{
    public:
        Node *root_node;
        unordered_map<string, Node *> * query_map;
        unordered_map<Node*, pair<int, int>> node_id_to_coords;
        int num_columns_per_page;

        TreeVisualizer(Node *root_node_, unordered_map<std::string, Node *> *query_map_, int num_columns_per_page_){
            this->root_node = root_node_;
            this->query_map = query_map_;
            this->num_columns_per_page = num_columns_per_page_;
        };
        string visualize_tree();
        string visualize_cost_str();

    private:
        void draw_lines_between_nodes(vector<vector<char>> *frame_p); // assumes that node coords
        unordered_map<string, pair<int, int>> node_id_to_pos; // node id maps to left most placement on graph
        int draw_node_and_set_coords(Node *node, vector<vector<char>> *frame, int local_depth, int width, int total_depth);
        vector<vector<char>> generate_output_frame();
};

#endif //TREE_VISUALIZER_H