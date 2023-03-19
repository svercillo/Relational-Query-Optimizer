#ifndef NODE_H
#define NODE_H

#include "state.h"

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class Node{

    public:
        
        virtual ~Node() {}  // make the destructor virtual to ensure correct object deletion

        virtual string to_string() const = 0;

        virtual string get_type() const = 0;

        int cost_of_operation = INT_MAX; // the cost in number of I/Os if applicable

        string id; // name, i.e. op1
        Node *parent = nullptr;
        string operation_descr;
        State state;


        Node(string _id)
        {
            this->id = _id;
        }


        Node(){}
};

class Selection : public Node{

    public:
        Selection(string _id) : Node(_id){

        }

        Selection(Selection* other, string input_relation, bool left){ // does not copy state!
            if (left)
                this->id = "PUSHED_" + other->id + "_LEFT";
            else
                this->id = "PUSHED_" + other->id + "_RIGHT";
            this->column_name = get_actual_col_name(other->column_name);
            this->comparison_operator = other->comparison_operator;
            this->comparison_value = other->comparison_value;    
        }

        const string type = "selection";
        Node *child; // either base or temp table
        string column_name; // can be base column of A.col_name
        string comparison_operator;
        string input_relation; // can be base or temp
        int comparison_value;

        string to_string() const override { // shallow to string methods
            string res = "Selection " + this->id + ":\n";
            res += "\tInput Relation: " + input_relation + "\n";
            res += "\tColumn Name: " + column_name + "\n";
            res += "\tComparison Operator: " + comparison_operator + "\n";
            res += "\tComparison Value: " + std::to_string(comparison_value) + "\n";
            // res += "\tChild Name: " + this->child->id + "\n";
            return res;
        }

        string get_type() const override {
            return this->type;
        }

    private:
        string get_actual_col_name(string conglomerate_col_string){
            string actual_col_name;
            if (conglomerate_col_string.find(".") != string::npos){
                actual_col_name = conglomerate_col_string.substr(conglomerate_col_string.find(".") + 1);
            } else {
                actual_col_name = conglomerate_col_string;
            }

            return actual_col_name;
        }
};

class Projection : public Node
{

    public: 
        Projection(string _id) : Node(_id){
        }

        const string type = "projection";
        
        // projected columns aliases (can contain OP1.column_name instead of column_name)
        vector<string> column_names; 
        
        string input_relation;  // can be base or temp
        Node *child; // input relation nodes

        string to_string() const override{ // shallow to string methods
            std::string res = "Projection: " + this->id + "\n";
            res += "\tInput Relation: " + input_relation + "\n";
            res += "\tColumns: ";
            for (const auto& col : column_names) {
                res += col + ", ";
            }
            res.pop_back(); // Remove trailing comma
            res.pop_back(); // Remove space after last column
            res += "\n";
            // res += "\tChild Name: " + this->child->id + "\n";
            return res;
        }

        string get_type() const override {
            return this->type;
        }
};

class Join : public Node
{
    public:
        Join(string _id) : Node(_id){  
        }

        const string type = "join";
        string left_table_name;
        string right_table_name;
        string left_join_on_col;
        string right_join_on_col;

        Node *left_child;
        Node *right_child;

        string to_string() const override { // shallow to string methods
            string res = "Join: " + this->id + "\n";
            res += "\tLeft Table: " + left_table_name + "\n";
            res += "\tRight Table: " + right_table_name + "\n";
            res += "\tLeft Join On Column: " + left_join_on_col + "\n";
            res += "\tRight Join On Column: " + right_join_on_col + "\n";
            return res;
        }

        string get_type() const override {
            return this->type;
        }
};

class BaseTable : public Node
{
    public:
        BaseTable(string _id) : Node(_id){
            this->table_name = _id;
        }
        const string type = "table";
        string table_name;

        string to_string() const override{ // shallow to string methods
            std::string res = "Base Table: " + this->id + "\n";
            return res;
        }

        string get_type() const override {
            return this->type;
        }
};

#endif // NODE_H