#include "relational_schema_parser.h"


std::unordered_map<std::string, ColumnTypes> column_types_map = {
    {"INT", COLUMNTYPES_INT},
    {"FLOAT", COLUMNTYPES_FLOAT},
    {"VARCHAR", COLUMNTYPES_VARCHAR},
    {"BYTE", COLUMNTYPES_BYTE},
    {"TEXT", COLUMNTYPES_TEXT}
};

std::string toupper(std::string value){
    std::string upper = "";
    for (int x = 0; x < value.length(); x++)
        upper += std::toupper(value[x]);

    return upper;
}

void RelationalSchemaParser::release_memory(){
    std::unordered_map<std::string, std::vector<ForeignKey *> > foreign_keys = this->schema->foreign_keys;
    for (auto it = foreign_keys.begin(); it != foreign_keys.end(); it++)
    {
        ForeignKey * fk = it->second[0];
        delete fk;
    }

    
    for (auto it = schema->tables.begin(); it != schema->tables.end(); it++) {
        Table * table = it->second;

        for (auto it2 = table->column_vals.begin(); it2!= table->column_vals.end(); it2++){
            Column *col = *it2;
            delete col;
        }

        delete table->primary_key;
        delete table;
    }

    delete this->schema;
}

RelationalSchemaParser::~RelationalSchemaParser(){
    release_memory();
}

void RelationalSchemaParser::fill_data_strucuture(){
    std::string contents = this->contents;
    std::string buffer = "";

    Schema *schema = new Schema();

    int word_start = 0;
    int word_end = 0;
    WordType last_word_type = _none;
    Table *table = nullptr;

    bool is_foreign_key = false;
    bool is_primary_key = false;
    bool is_foreign_key_reference_key = false;
    bool possible_end_of_primary_key = false;
    bool possible_end_of_foreign_key_reference_key = false;
    bool new_foreign_key = false;
    bool new_primary_key = false;

    bool is_table_statement = false;
    bool new_foreign_key_reference_key = false;
    std::vector<std::string> column_vals; 
    std::vector<std::string> primary_key_col_names;
    std::vector<std::string> foreign_key_table_names;
    std::vector<std::string> foreign_key_reference_table_names;
    std::vector<std::vector<std::string> > foreign_key_col_names;            // can have multiple foreign keys per tabl
    std::vector<std::vector<std::string> > foreign_key_reference_col_names; // can have multiple foreign keys per tabl

    for (int i = 0; i < contents.length(); i++)
    {
        if (
            contents[i] == '\n' || contents[i] == '\r' || contents[i] == '\t' || contents[i] == ' ' || contents[i] == ',' || contents[i] == '(' || contents[i] == ')')
        {
            if (word_end - word_start > 0){
                std::string word = toupper(contents.substr(word_start, word_end - word_start+1));
                
                switch (last_word_type)
                {
                case _none:
                {
                    if (word.compare("TABLE") == 0){
                        last_word_type = _table;
                    } else if (word.compare("FOREIGN") == 0) {
                        last_word_type = _foreign;
                    }
                    break;
                }
                case _table:
                {
                    // TODO: check protected words
                    is_table_statement = true;
                    table = new Table();
                    table->table_name = word;
                    last_word_type = _table_name;
                    break;
                }
                case _table_name:
                case _column_name:
                {
                    if (word.compare("PRIMARY") == 0){
                        last_word_type = _primary;
                    } else if (word.compare("FOREIGN") == 0){
                        last_word_type = _foreign;
                    } else if (word.compare("REFERENCES") == 0){
                        if (last_word_type == _table_name){
                            std::cout << "REFERENCES WORD SHOULD ONLY PRECEED TABLE NAME"; 
                            abort();
                        }

                        is_foreign_key = false;
                        is_foreign_key_reference_key = true;
                        last_word_type = _references;
                    } else if (is_primary_key){
                        primary_key_col_names.push_back(word);
                        

                        last_word_type = _column_name;
                    } else if (is_foreign_key){ 
                        if (new_foreign_key){
                            std::vector<std::string> key_cols;
                            foreign_key_col_names.push_back(key_cols);
                            new_foreign_key = false;
                        }

                        foreign_key_col_names[foreign_key_col_names.size() -1].push_back(word);
                        last_word_type = _column_name;

                    } else if (is_foreign_key_reference_key){

                        if (new_foreign_key_reference_key){
                            std::vector<std::string> ref_key_cols;
                            foreign_key_reference_col_names.push_back(ref_key_cols);
                            new_foreign_key_reference_key = false;
                        }

                        foreign_key_reference_col_names[foreign_key_reference_col_names.size() - 1].push_back(word);

                        possible_end_of_foreign_key_reference_key = true;
                        last_word_type = _column_name;
                    } else { // normal column
                        // TODO: check invalid column name
                        column_vals.push_back(word);
                        last_word_type = _column_name;
                    }
                    break;
                }
                case _primary:
                {
                    if (word.compare("KEY") != 0)
                    {
                        std::cout << "PRIMARY WORD MUST PRECEED KEY " << std ::endl;
                        abort();
                    }

                    last_word_type = _primary_key;
                    break;
                }
                case _primary_key:
                {
                    // TODO: check protected words
                    primary_key_col_names.push_back(word);
                    is_primary_key = true;
                    last_word_type = _column_name;
                    break;
                }
                case _foreign:
                {
                    if (word.compare("KEY") != 0){
                        std::cout << "FOREIGN WORD MUST PRECEED KEY " << std ::endl;
                        abort();
                    }
                    new_foreign_key = true;
                    is_foreign_key = true;
                    last_word_type = _column_name;
                    break;
                }
                case _references: 
                {
                    foreign_key_reference_table_names.push_back(word);
                    
                    is_foreign_key_reference_key = true;
                    new_foreign_key_reference_key = true;

                    last_word_type = _column_name;
                    break;
                }
                default:
                    break;
                }
            }

            word_start = i + 1;
        }
        else if (contents[i] == ';')
        {
            if (is_table_statement)
            {
                process_table_statemeent(
                    primary_key_col_names,
                    table,
                    schema,
                    column_vals,
                    foreign_key_reference_table_names,
                    foreign_key_col_names,
                    foreign_key_reference_col_names
                );

                is_table_statement = false;
            } else {
                process_foriegn_key_statement(
                    primary_key_col_names,
                    table,
                    schema,
                    foreign_key_reference_table_names,
                    foreign_key_col_names,
                    foreign_key_reference_col_names
                );
            }

            column_vals.clear();
            primary_key_col_names.clear();
            foreign_key_table_names.clear();
            foreign_key_reference_table_names.clear();
            foreign_key_col_names.clear();
            foreign_key_reference_col_names.clear();

            last_word_type = _none;
            word_start = i + 1;
        }
        else
        {
            word_end = i;
        }

        if (contents[i] == ')'){
            if (is_primary_key)
            {
                is_primary_key = false;
            }
            else if (possible_end_of_foreign_key_reference_key)
            {
                possible_end_of_foreign_key_reference_key = false;
                is_foreign_key_reference_key = false;
            }
        }
    }

    this->schema = schema;
}

void RelationalSchemaParser::process_table_statemeent(
    std::vector<std::string> primary_key_col_names,
    Table * table,
    Schema * schema,
    std::vector<std::string> column_vals,
    std::vector<std::string> foreign_key_reference_table_names,
    std::vector<std::vector<std::string> > foreign_key_col_names,
    std::vector<std::vector<std::string> > foreign_key_reference_col_names
){

    std::string last_col_name = "";
    for (std::string col_name : column_vals)
    {   
        if (column_types_map.find(col_name) != column_types_map.end()){
            if (column_types_map.find(last_col_name) != column_types_map.end()){
                std::cout << "INVALID COLUMN NAMES, USE OF PROTECTED STRINGS" << std::endl;
            }

            ColumnTypes col_type = column_types_map[col_name];
            Column *column = new Column(last_col_name, col_type);
            table->column_vals.push_back(column);
            table->columns[column->column_name] = column;
        }
        else if (last_col_name.compare("") != 0 && column_types_map.find(last_col_name) == column_types_map.end())
        {
            Column *column = new Column(last_col_name);
            table->column_vals.push_back(column);
            table->columns[column->column_name] = column;
        }
        last_col_name = col_name;
    }

    if (column_types_map.find(last_col_name) == column_types_map.end()){
        Column *column = new Column(last_col_name);
        table->column_vals.push_back(column);
        table->columns[column->column_name] = column;
    }


    PrimaryKey *primary_key = new PrimaryKey();
    if (primary_key_col_names.size() > 0)
        primary_key->is_present = true;

    for (std::string col_name : primary_key_col_names)
    {

        std::unordered_map<std::string, Column *>::iterator found = table->columns.find(col_name);

        if (found == table->columns.end())
        {
            std::cout << "PRIMARY KEY COL " + col_name + " IS NOT A VALID COLUMN" << std::endl;
            abort();
        }

        primary_key->key_columns.push_back(found->second);
    }

    table->primary_key = primary_key;

    schema->tables[table->table_name] = table;
    schema->table_vals.push_back(table);

    int num_foreign_keys = foreign_key_reference_table_names.size();
    if (num_foreign_keys != foreign_key_col_names.size() || foreign_key_col_names.size() != foreign_key_reference_col_names.size()){
        std::cout << "FOREIGN KEYS MUST HAVE THE SAME NUMBER OF COLUMNS AS ITS REFERNCE KEY";
        abort();
    }
    
    for (int ind = 0; ind < num_foreign_keys; ind++){
        ForeignKey * foreign_key =  new ForeignKey();
        std::vector<std::string> key_cols = foreign_key_col_names[ind];
        std::vector<std::string> reference_cols = foreign_key_reference_col_names[ind];

        std::string reference_table_name = foreign_key_reference_table_names[ind];
        std::string table_name = table->table_name;

        for (std::string col_name : key_cols)
        {
            std::unordered_map<std::string, Column *>::iterator found = table->columns.find(col_name);

            if (found == table->columns.end())
            {
                std::cout << "FOREIGN KEY COL " + col_name + " IS NOT A VALID COLUMN";
                abort();
            }
            foreign_key->key_columns.push_back(found->second);
        }

        if (schema->tables.find(reference_table_name) == schema->tables.end())
        {
            std::cout << "FOREIGN KEY REFERENCES TABLE WHICH DOES NOT EXIST";
            abort();
        }

        Table *reference_table = schema->tables.find(reference_table_name)->second;

        for (std::string col_name : reference_cols)
        {
            std::unordered_map<std::string, Column *>::iterator found = reference_table->columns.find(col_name);

            if (found == table->columns.end())
            {
                std::cout << "FOREIGN KEY COL " + col_name + " IS NOT A VALID COLUMN";
                abort();
            }
            foreign_key->reference_columns.push_back(found->second);
        }

        if (schema->foreign_keys.find(table_name) == schema->foreign_keys.end()){
            std::vector<ForeignKey *> foreign_keys;
            schema->foreign_keys.insert(std::make_pair(table_name, foreign_keys));
        }


        foreign_key->table = table;
        foreign_key->reference_table = reference_table;

        schema->foreign_keys.find(table_name)->second.push_back(foreign_key);
    }
}

void RelationalSchemaParser::process_foriegn_key_statement(
    std::vector<std::string> primary_key_col_names,
    Table * table,
    Schema * schema,
    std::vector<std::string> foreign_key_reference_table_names,
    std::vector<std::vector<std::string> > foreign_key_col_names,
    std::vector<std::vector<std::string> > foreign_key_reference_col_names
){
    if (foreign_key_col_names[0].size() - 1 != foreign_key_reference_col_names[0].size())
    {
        std::cout << "FOREIGN KEYS MUST HAVE THE SAME NUMBER OF COLUMNS AS ITS REFERNCE KEY";
        abort();
    }

    int ind = 0;
    ForeignKey * foreign_key =  new ForeignKey();
    std::vector<std::string> key_cols = foreign_key_col_names[ind];
    std::vector<std::string> reference_cols = foreign_key_reference_col_names[ind];

    std::string reference_table_name = foreign_key_reference_table_names[ind];
    std::string table_name = key_cols[0];                
    
    if (schema->tables.find(table_name) == schema->tables.end())
    {
        std::cout << "FOREIGN KEY ON TABLE THAT DOESN'T EXIST";
        abort();
    }

    Table * pTable = schema->tables.find(table_name)->second;
    
    for (int i = 1; i < key_cols.size(); i++)
    {
        std::string col_name = key_cols[i];
        std::unordered_map<std::string, Column *>::iterator found = pTable->columns.find(col_name);
        if (found == pTable->columns.end())
        {
            std::cout << "FOREIGN KEY COL " + col_name + " IS NOT A VALID COLUMN";
            abort();
        }
        foreign_key->key_columns.push_back(found->second);
    }

    if (schema->tables.find(reference_table_name) == schema->tables.end())
    {
        std::cout << "FOREIGN KEY REFERENCES TABLE WHICH DOES NOT EXIST";
        abort();
    }

    Table *reference_table = schema->tables.find(reference_table_name)->second;

    for (std::string col_name : reference_cols)
    {
        std::unordered_map<std::string, Column *>::iterator found = reference_table->columns.find(col_name);

        if (found == pTable->columns.end())
        {
            std::cout << "FOREIGN KEY COL " + col_name + " IS NOT A VALID COLUMN";
            abort();
        }
        foreign_key->reference_columns.push_back(found->second);
    }

    if (schema->foreign_keys.find(table_name) == schema->foreign_keys.end()){
        std::vector<ForeignKey *> foreign_keys;
        schema->foreign_keys.insert(std::make_pair(table_name, foreign_keys));
    }

    foreign_key->table = pTable;
    foreign_key->reference_table = reference_table;

    schema->foreign_keys.find(table_name)->second.push_back(foreign_key);
}