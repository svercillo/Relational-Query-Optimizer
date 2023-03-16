#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>
#include <unordered_map>


using namespace std;

class Node{


    public:
        
        virtual ~Node() {}  // make the destructor virtual to ensure correct object deletion

        virtual string to_string() const = 0;

        virtual string get_type() const = 0;


        string id; // name, i.e. op1
        Node *parent;

        Node(string _id)
        {
            this->id = _id;
        }
};

class Selection : public Node{

    public:
        Selection(string _id) : Node(_id){

        }
        const string type = "selection";
        Node *child; // either base or temp table
        string column_name;
        string comparison_operator;
        string input_relation; // can be base or temp
        int comparison_value;

        string to_string() const override { // shallow to string methods
            string res = "Selection " + this->id + ":\n";
            res += "\tInput Relation: " + input_relation + "\n";
            res += "\tColumn Name: " + column_name + "\n";
            res += "\tComparison Operator: " + comparison_operator + "\n";
            res += "\tComparison Value: " + std::to_string(comparison_value) + "\n";
            return res;
        }

        string get_type() const override {
            return this->type;
        }
};

class Projection : public Node
{

    public: 
        Projection(string _id) : Node(_id){
        }

        const string type = "projection";
        vector<string> column_names; // projected columns
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


class BaseTable : Node
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