#ifndef ENUMS_H
#define ENUMS_H

enum SchemaWordTypes
{
    _table, // 0
    _table_name, // 1
    _column_name, // 2
    _column_type, // 3
    _foreign, // 4
    _foreign_key, // 5
    _references, // 6
    _primary, // 7
    _primary_key, // 8
    _none, // 9
    _null, // 10
    _not, // 11
    _default, // 12
}; // list of all the possible types of words in a SQL schema

enum ParserWordTypes
{
    _op_relation_name, //0 
    _input_relation, //1
    _operation_equals, //2

    _selection, //3
    _select_col_name,//4
    _select_col_val, //5s
    _select_comparision, // i.e. <, <=, >, >=, = // 6

    _projection, // 7
    _projection_col_name, //8

    _join, //9
    _joined_on, //10
    _join_table, //11
    _join_col_name1, //12
    _join_eqs, //13
    _join_col_name2, //14

    _cardiality, //15
    _size, // 16
    _rf, // 17
    _range, // 18
    _height, // 19

    _statistic_subject, // 20
    _statistic_subject_end, // 21
    _statistic_equals, //22

    _statistic_values, //23
    _none_, // 24
    _in_ // 25
}; // list of all the possible types of words in a SQL schema

enum StatisticType
{
    STATISTICTYPE_CARDINALITY,
    STATISTICTYPE_SIZE,
    STATISTICTYPE_RF,
    STATISTICTYPE_RANGE,
    STATISTICTYPE_HEIGHT
};

#endif // WORDTYPE_H


