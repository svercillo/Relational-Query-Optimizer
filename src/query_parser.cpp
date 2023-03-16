#include "query_parser.h"

void QueryParser::set_parent_and_child_pointers(){
    unordered_map<string, Node *> * output_map_p = &this->output_map;

    cout << "OUTPUT SIZE " << (*output_map_p).size() << endl;
    for (auto entry : (*output_map_p))
    {
        Node *node = entry.second;
        if (node->get_type() == "selection"){
            Selection *selection = dynamic_cast<Selection *>(node);
            string input_relation = selection->input_relation;

            Node *child_node = (*output_map_p)[input_relation];
            if (child_node == nullptr){
                // if there are multiple uses of table A, it is represented as multiple new objects
                child_node = (Node*) new BaseTable(input_relation); // potentially duplicated base table
            }

            child_node->parent = node; // set child_node parent pointer
        }
        else if (node->get_type() == "projection")
        {
            Projection *projection = dynamic_cast<Projection *>(node);
            string input_relation = projection->input_relation;

            Node *child_node = (*output_map_p)[input_relation];
            if (child_node == nullptr){
                // if there are multiple uses of table A, it is represented as multiple new objects
                child_node = (Node*) new BaseTable(input_relation); // potentially duplicated base table
            }

            child_node->parent = node; // set child_node parent pointer
        }
        else if (node->get_type() == "join")
        {
            Join *join = dynamic_cast<Join *>(node);

            string left_table_name = join->left_table_name;
            string right_table_name = join->right_table_name;

            Node *left_child_node = (*output_map_p)[left_table_name];
            if (left_child_node == nullptr){
                // if there are multiple uses of table A, it is represented as multiple new objects
                left_child_node = (Node*) new BaseTable(left_table_name); // potentially duplicated base table
            }

            Node *right_child_node = (*output_map_p)[right_table_name];
            if (right_child_node == nullptr){
                // if there are multiple uses of table A, it is represented as multiple new objects
                right_child_node = (Node*) new BaseTable(right_table_name); // potentially duplicated base table
            }

            join->left_child = left_child_node;
            join->right_child = right_child_node;

            left_child_node->parent = join; // set child_node parent pointer
            right_child_node->parent = join; // set child_node parent pointer

        }
    }
};


void QueryParser::fill_data_structures()
{
    // Copy the contents and output_map_p member pointer for easier access
    std::string contents = this->contents;
    unordered_map<std::string, Node *>* output_map_p = &(this->output_map);

    // Initialize variables for word tracking and parsing
    int word_start = 0;
    int word_end = 0;
    ParserWordTypes last_word_type = _none_;

    // Initialize variables for storing parsed SQL statements
    std::string resultant_relation;
    std::string input_relation;
    std::string selection_col_name;
    std::string selection_comparision = "";
    std::string selection_col_value;
    std::vector<std::string> projection_cols;
    std::string joined_table;
    std::string joined_col_name1; 
    std::string joined_col_name2; 

    // Loop through each character in the SQL query
    for (int i = 0; i < contents.length(); i++)
    {
        // Check for whitespace or other delimiters that mark the end of a word
        if (
            contents[i] == '\n' 
            || contents[i] == '\r' 
            || contents[i] == '\t' 
            || contents[i] == ' ' 
            || contents[i] == ',' 
            || contents[i] == '('
            || contents[i] == ')'
        ){
            // If a word has been fully read in, process it
            if (word_end - word_start >= 0){
                
                // Get the current word, and check if it is an operator or keyword
                std::string word = toupper(contents.substr(word_start, word_end - word_start+1));
                if (word.rfind("OP", 0) == 0)
                {
                    switch (last_word_type)
                    {
                    case _none_:
                        last_word_type = _op_relation_name;
                        resultant_relation = word;
                        break;
                    case _operation_equals:
                        input_relation = word;
                        last_word_type = _input_relation;
                        break;
                    default:
                        break;
                    }
                }
                else if (word == "RESULT") {
                    switch (last_word_type)
                    {
                    case _none_:
                        last_word_type = _op_relation_name;
                        resultant_relation = word;
                        break;
                    default:
                        break;
                    }
                }
                // If the current word is none of the above, it must be a column name, value, or keyword
                else
                {
                    switch (last_word_type)
                    {
                    case _op_relation_name:
                    {
                        if (word != "="){
                            std::cout << "INVALID STATEMENT" << std::endl;
                            std::cout << word << std::endl;
                            abort();
                        }

                        last_word_type = _operation_equals;
                        break;
                    }
                    case _operation_equals:
                    {
                        input_relation = word;
                        last_word_type = _input_relation;
                        break;
                    }
                    case _input_relation:
                    {
                        if (word == "SELECTION") {
                            last_word_type = _selection;
                        } else if (word == "PROJECTION") {
                            last_word_type = _projection;
                        } else if (word == "JOIN") {
                            last_word_type = _join;
                        } else {
                            std::cout << "INVALID SQL OPERATION" << std::endl;
                            abort();
                        }
                        break;
                    }
                    case _selection: 
                    {
                        selection_col_name = word;
                        last_word_type = _select_col_name;
                        break;
                    }
                    case _select_comparision:
                    {
                        selection_col_value = word;
                        last_word_type = _select_col_val;
                        break;
                    }
                    case _projection:
                    case _projection_col_name:
                    {
                        projection_cols.push_back(word);
                        last_word_type = _projection_col_name;
                        break;
                    }
                    case _join:
                    {
                        joined_table = word;
                        last_word_type = _join_table;
                        break;
                    }
                    case _join_table:
                    {
                        cout << "join on  " << word << endl;
                        if (word != "ON"){
                            std::cout << "INVALID JOIN STATEMENT" << std::endl;
                            abort();
                        }
                        last_word_type = _joined_on;
                        break;
                    }
                    case _joined_on:
                    {
                        joined_col_name1 = word;
                        last_word_type = _join_col_name1;
                        break;
                    }
                    case _join_col_name1: 
                    {
                        if (word != "="){
                            std::cout << "INVALID JOIN STATEMENT" << std::endl;
                        }
                        last_word_type = _join_eqs;
                        break;
                    }
                    case _join_eqs:
                    {
                        joined_col_name2 = word;
                        last_word_type = _join_col_name2;
                        break;
                    }
                    default:
                        break;
                    }
                }
            }

            if (contents[i] == '\n')
            {
                switch (last_word_type)
                {
                case _select_col_val:
                {

                    
                    Selection *sel = new Selection(resultant_relation);
                    sel->column_name = selection_col_name;
                    sel->comparison_operator = selection_comparision;
                    sel->comparison_value = stoi(selection_col_value);
                    sel->input_relation = input_relation;

                    (*output_map_p)[resultant_relation] = sel;
                    cout << sel->to_string() << endl;

                    break;
                    }
                    case _projection_col_name:
                    {

                        Projection * proj = new Projection(resultant_relation);
                        vector<string> column_names_cpy(projection_cols);
                        proj->column_names = column_names_cpy;
                        proj->input_relation = input_relation;
                        (*output_map_p)[resultant_relation] = proj;

                        cout << proj->to_string() << endl;
                        projection_cols.clear(); // reset to null
                        break;
                    }
                    case _join_col_name2:
                    {
                        Join * join = new Join(resultant_relation);
                        join->left_table_name = resultant_relation;
                        join->right_table_name = joined_table;

                        // TODO parse col_names for A.a 
                        join->left_join_on_col = joined_col_name1;
                        join->right_join_on_col = joined_col_name2;
                        

                        (*output_map_p)[resultant_relation] = join;
                        cout << join->to_string() << endl;

                        break;
                    }
                    default: {
                        break;
                    }
                    }
                last_word_type = _none_;
            } 
            word_start = i + 1;
        }
        else if (
            (
                contents[i] == '='
                || contents[i] == '<'
                || contents[i] == '>'
            ) && (
                last_word_type == _select_col_name
                || last_word_type == _select_comparision
            )
        ){
            selection_comparision += contents[i];
            last_word_type = _select_comparision;
            word_start = i + 1;
        }
        else if (contents[i] == '=' && last_word_type == _joined_on)
        { // equijoin table1
            std::string word = toupper(contents.substr(word_start, word_end - word_start + 1));
            joined_col_name1 = word;

            last_word_type = _join_eqs;
            word_start = i + 1;
        }
        else
        {
            word_end = i;
        }
    }
}
