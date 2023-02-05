#include "relational_schema_parser.h"




std::string toupper(std::string value){
    std::string upper = "";
    for (int x = 0; x < value.length(); x++)
        upper += std::toupper(value[x]);

    return upper;
}

Schema RelationalSchemaParser::fill_data_strucuture(){
    std::string contents = this->contents;
    std::string buffer = "";

    Schema schema = Schema();

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
    bool is_table_statement = false;
    for (int i = 0; i < contents.length(); i++)
    {
        if (
            contents[i] == '\n' 
            || contents[i] == '\r' 
            || contents[i] == '\t' 
            || contents[i] == ' ' 
            || contents[i] == ',' 
            || contents[i] == '('
        ){
            if (word_end - word_start > 0){
                std::string word = toupper(contents.substr(word_start, word_end - word_start+1));

                std::cout << word << std::endl;
                
                switch (last_word_type)
                {
                case _none:
                {
                    if (word.compare("TABLE") == 0){
                        last_word_type = _table;
                    } else if (word.compare("FOREIGN")) {
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

                    std::cout << word << std::endl;

                    if (word.compare("PRIMARY") == 0){
                        last_word_type = _primary;
                    } else if (word.compare("FOREIGN") == 0){
                        last_word_type = _foreign;
                    } else if (word.compare("REFERENCES") == 0){
                        if (last_word_type == _table_name){
                            std::cout << "INVALID"; 
                            abort();
                        }

                        is_foreign_key = false;
                        is_foreign_key_reference_key = true;
                        last_word_type = _references;
                    } else if (is_primary_key){
                        Column key_column = Column(word); // column name
                        table->primary_key.key_columns.push_back(key_column);
                        
                        possible_end_of_primary_key = true;
                    } else if (is_foreign_key){
                        Column key_column = Column(word); // column name
                        if (new_foreign_key){
                            ForeignKey foreign_key = ForeignKey();
                            foreign_key.key_columns.push_back(key_column);
                            table->foreign_keys.push_back(foreign_key);
                            new_foreign_key = false;
                        } else { 
                            // add column to foreign key for multi column forgien key
                            table->foreign_keys[table->foreign_keys.size() - 1].key_columns.push_back(key_column);
                        }
                    } else if (is_foreign_key_reference_key){
                        Column reference_column = Column(word); // column name
                        table->foreign_keys[table->table_name.size() - 1].reference_columns.push_back(reference_column);
                        
                        possible_end_of_foreign_key_reference_key = true;
                    } else { // normal column
                        // check invalid column name

                        
                        Column column = Column(word);
                        table->columns.push_back(column);

                        std::cout << "HEREE column size "  << table->columns.size() << std::endl;
                    }

                    last_word_type = _column_name;
                    break;
                }
                case _primary:
                {
                    if (word.compare("KEY") != 0){
                        std::cout << "INVALID ";
                        abort();
                    }
                    
                    last_word_type = _primary_key;
                    break;
                }
                case _primary_key:
                {
                    // TODO: check protected words
                    PrimaryKey primary_key = PrimaryKey();
                    Column key_column = Column(word);
                    primary_key.key_columns.push_back(key_column);


                    is_primary_key = true;
                    last_word_type = _column_name;
                    break;
                }
                case _foreign:
                {
                    if (word.compare("KEY") != 0){
                        std::cout << "INVALID ";
                        abort();
                    }
                    new_foreign_key = true;
                    is_foreign_key = true;
                    last_word_type = _column_name;
                    break;
                }

                case _references: 
                {
                    std::string reference_table_name = word;

                    if (schema.tables.find(reference_table_name) == schema.tables.end()){
                        // reference table in foreign key must already exist
                        std::cout << "INVALID";
                        abort();
                    }

                    table->foreign_keys[table->foreign_keys.size() - 1].reference_table_name = reference_table_name;
                    is_foreign_key_reference_key = true;

                    last_word_type = _column_name;
                    break;
                }
                default:
                    break;
                }
            } else{ 
                last_word_type = _none;
            }

            word_start = i + 1;
        }
        else if (contents[i] == ';' )
        {
            if (is_table_statement){
                schema.tables[table->table_name] = *table;
                is_table_statement = false;
            }
            word_start = i + 1;
        }
        else
        {
            if (contents[i] == ')'){
                if (possible_end_of_primary_key){
                    possible_end_of_primary_key = false; 
                    is_primary_key = false;
                } else if (possible_end_of_foreign_key_reference_key) {
                    possible_end_of_foreign_key_reference_key = false;
                    is_foreign_key_reference_key = false;
                }
            }
            word_end = i;
        }
    }


    std::cout << "column size "  << table->columns.size() << std::endl;
    return schema;
}