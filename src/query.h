#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <vector>
#include <unordered_map>


enum PossibleSelections
{
    POSSIBLESELECTIONS_EQUALS,
    POSSIBLESELECTIONS_LT,
    POSSIBLESELECTIONS_GT,
    POSSIBLESELECTIONS_LTE,
    POSSIBLESELECTIONS_GTE
};

// std::unordered_map<std::string, PossibleSelections> column_types_map = {
//     {"=", POSSIBLESELECTIONS_EQUALS},
//     {"<", POSSIBLESELECTIONS_LT},
//     {">", POSSIBLESELECTIONS_GT},
//     {"<=", POSSIBLESELECTIONS_LTE},
//     {">=", POSSIBLESELECTIONS_GTE}
// };

// struct Query{
//     public:
//         std::string resultant_relation;
//         Query() {}
// };

// struct Selection : Query {
//     public:
//         std::string input_relation;
//         PossibleSelections selection_criterion;
//         std::string column_name;
//         std::string column_value;
// };

// struct Projection : Query{
//     public:
//         std::string input_relation;
//         std::vector<std::string> column_names;
// };

// struct EquiJoin : Query
// {
//     public:
//         std::string input_relation1;
//         std::string input_relation2;
//         std::string col_from_input_relation1;
//         std::string col_from_input_relation2;
// };

#endif // QUERY_H