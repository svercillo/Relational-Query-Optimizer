#include "statistics_parser.h"

std::pair<int, int> parse_range_values(std::vector<std::string> token_arr) {
    int start_range, end_range;
    std::string start_substr = token_arr[0];
    std::string end_substr = token_arr[1];

    try
    {
        start_range = std::stoi(start_substr);
        end_range = std::stoi(end_substr);
    }
    catch (const std::exception &e)
    {
        std::cout << "INVALID STATISTIC STATEMENT" << std::endl;
        abort();
    }

    return std::make_pair(start_range, end_range);
};

std::string stats_to_string(const std::unordered_map<std::string, Statistics*>& statistics_per_table) {
    std::string res;
    for (auto it = statistics_per_table.begin(); it != statistics_per_table.end(); ++it) {
        const std::string& table_name = it->first;
        Statistics* stats_ptr = it->second;
        res += table_name + "\n";
        res += stats_ptr->to_string();
        res += "\n\n";
    }
    return res;
};

void StatisticsParser::print_stats()
{

    std::string res;
    res = stats_to_string(this->statistics_per_table);

    std::cout << "\nPRINTING STATS FOR " << this->statistics_per_table.size() << " TABLES: \n"
              << endl;
    std::cout << res << std::endl;

    return;
};

void StatisticsParser::fill_data_structures(){
    std::string contents = this->contents;
    std::string buffer = "";

    int word_start = 0;
    int word_end = 0;
    ParserWordTypes last_word_type = _none_;

    bool multi_var_stat = false;
    std::string resultant_relation;
    std::string input_relation;
    std::vector<std::string> statistic_cols;
    std::string statistic_col;
    std::vector<std::string> statistic_values;
    std::vector<std::string> range_values;
    std::vector<std::string> statistic_subject_strs;
    StatisticType statistic_type;

    

    for (int i = 0; i < contents.length(); i++)
    {
        std::string word = toupper(contents.substr(word_start, word_end - word_start+1));
        

        if (
            contents[i] == '\n' || contents[i] == '\r' || contents[i] == '\t' || contents[i] == ' ' || contents[i] == ',' || contents[i] == '(' || contents[i] == ')')
        {
            if (word_end - word_start >= 0)
            {
                std::string word = toupper(contents.substr(word_start, word_end - word_start+1));

                switch (last_word_type)
                {
                    case _none_:
                    {
                        if (word == "CARDINALITY")
                        {
                            statistic_type = STATISTICTYPE_CARDINALITY;
                            last_word_type = _cardiality;
                        }
                        else if (word == "SIZE")
                        {
                            statistic_type = STATISTICTYPE_SIZE;
                            last_word_type = _size;
                        }
                        else if (word == "RF")
                        {
                            statistic_type = STATISTICTYPE_RF;
                            last_word_type = _rf;
                        }
                        else if (word == "HEIGHT")
                        {
                            statistic_type = STATISTICTYPE_HEIGHT;
                            last_word_type = _height;
                        }
                        else if (word == "RANGE")
                        {
                            statistic_type = STATISTICTYPE_RANGE;
                            last_word_type = _range;
                        }
                        break;
                    }
                    case _cardiality:
                    case _size:
                    case _rf: 
                    case _height:
                    case _range:
                    case _statistic_subject:
                    {
                        statistic_subject_strs.push_back(word);
                        last_word_type = _statistic_subject;
                        break;
                    }
                    case _statistic_subject_end:
                    {
                        last_word_type = _statistic_equals;
                        break;
                    }
                    case _statistic_equals:
                    case _statistic_values:
                    {
                        statistic_values.push_back(word);
                        last_word_type = _statistic_values;
                        break;
                    }
                    default:
                        break;
                }
            }

            if (contents[i] == '\n')
            {

                switch (last_word_type) {
                    case _statistic_values:
                    {


                        if (statistic_values.size() < 1)
                        {
                            std::cout << "INVALID STATISTIC LINE" << std::endl;
                            abort();
                        }

                        std::vector<std::string> token_arr = statistic_subject_strs;
                        if (token_arr.size() == 0) {
                            std::cout << "INVALID STATISTIC STATEMENT FORMAT" << std::endl;
                            abort();
                        }


                        std::string table_name = token_arr[token_arr.size() - 1];

                        if (token_arr.size() == 1) {
                            if (this->statistics_per_table.find(table_name) == this->statistics_per_table.end())
                            {
                                // new table statitics
                                Statistics *table_stats = new Statistics();
                                this->statistics_per_table[table_name] = table_stats;
                            }

                            switch (statistic_type) {
                                case STATISTICTYPE_CARDINALITY:
                                {
                                    this->statistics_per_table[table_name]->cardinality = std::stod(statistic_values[0]);
                                    break;
                                }
                                case STATISTICTYPE_SIZE:
                                {
                                    this->statistics_per_table[table_name]->size = std::stod(statistic_values[0]);
                                    break;
                                }
                                default:
                                {
                                    break;
                                }
                            }
                        } else if (token_arr.size() > 1){
                            std::string table_name;
                            std::vector<std::string> col_names;
                            std::vector<std::string> col_vals;


                            if (token_arr[1] == "IN"){
                                for (int i = 0; i < token_arr.size() - 2; i++)
                                {
                                    col_names.push_back(token_arr[i]);
                                }
                                table_name = token_arr[token_arr.size() - 1];
                                col_vals = statistic_values;
                            }
                            else
                            {
                                for (int i = 0; i < token_arr.size(); i++)
                                {
                                    col_names.push_back(token_arr[i]);
                                }
                                table_name = statistic_values[0];
                                for (int i = 2; i < statistic_values.size(); i++){
                                    col_vals.push_back(statistic_values[i]);
                                }
                            }   

                            if (this->statistics_per_table.find(table_name) == this->statistics_per_table.end())
                            {
                                // new table statitics
                                Statistics *table_stats = new Statistics();
                                this->statistics_per_table.insert(make_pair(table_name, table_stats));
                            }
                            

                            switch (statistic_type)
                            {
                                case STATISTICTYPE_CARDINALITY:
                                {
                                    this->statistics_per_table[table_name]->column_tup_cardinalities.insert(
                                        make_pair(col_names, std::stod(col_vals[0])));

                                    break;
                                }
                                case STATISTICTYPE_SIZE:
                                {
                                    this->statistics_per_table[table_name]->column_tup_sizes.insert(
                                        make_pair(col_names, std::stod(col_vals[0])));
                                    break;
                                }
                                case STATISTICTYPE_RF:
                                {
                                    this->statistics_per_table[table_name]->column_tup_rf.insert(
                                        make_pair(col_names, std::stof(col_vals[0])));
                                break;
                                }
                                case STATISTICTYPE_RANGE:
                                {
                                    std::pair<int, int> range = parse_range_values(col_vals);
                                    
                                    this->statistics_per_table[table_name]->column_range.insert(
                                        make_pair(col_names[0], std::vector<int>{range.first, range.second}));
                                    
                                    break;
                                }
                                case STATISTICTYPE_HEIGHT:
                                {
                                    this->statistics_per_table[table_name]->column_tup_index_height.insert(
                                        make_pair(col_names, std::stoi(col_vals[0])));

                                    break;
                                }
                                default:
                                {
                                    break;
                                }
                            }
                        }

                        statistic_subject_strs.clear();
                        statistic_values.clear();

                        break;
                    }
                    default: {
                        break;
                    }
                }
                last_word_type = _none_;
            } 
            else if (contents[i] == ')' && last_word_type == _statistic_subject)
            {
                last_word_type = _statistic_subject_end;
            }
            word_start = i + 1;
        }
        else
        {
            word_end = i;
        }
    }
};
