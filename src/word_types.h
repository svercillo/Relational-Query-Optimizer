#ifndef WORDTYPE_H
#define WORDTYPE_H

enum WordType
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


#endif // WORDTYPE_H


