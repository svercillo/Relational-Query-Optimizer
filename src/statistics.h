#ifndef STATISTICS_H
#define STATISTICS_H

#include "enums.h"
#include "utils.h"

#include <sstream>
#include <string>
#include <unordered_map>

struct Statistics{

    Statistics(){};

    double cardinality;
    double size;

    std::unordered_map<std::vector<std::string>, double, vector_hash, vector_equal> column_tup_cardinalities;
    std::unordered_map<std::vector<std::string>, double, vector_hash, vector_equal> column_tup_sizes;
    std::unordered_map<std::vector<std::string>, float, vector_hash, vector_equal> column_tup_rf;
    std::unordered_map<std::vector<std::string>, int, vector_hash, vector_equal> column_tup_index_height;
    std::unordered_map<std::string, std::vector<int> > column_range;

    std::string to_string() {
        std::ostringstream oss;
        oss << "Cardinality: " << this->cardinality << std::endl;
        oss << "Size: " << this->size << std::endl;
        oss << "Column tuple cardinalities: " << std::endl;
        for (const auto& entry : this->column_tup_cardinalities) {
            oss << "  " << vector_to_string(entry.first) << ": " << entry.second << std::endl;
        }
        oss << "Column tuple sizes: " << std::endl;
        for (const auto& entry : this->column_tup_sizes) {
            oss << "  " << vector_to_string(entry.first) << ": " << entry.second << std::endl;
        }
        oss << "Column tuple RF: " << std::endl;
        for (const auto& entry : this->column_tup_rf) {
            oss << "  " << vector_to_string(entry.first) << ": " << entry.second << std::endl;
        }
        oss << "Column tuple index height: " << std::endl;
        for (const auto& entry : this->column_tup_index_height) {
            oss << "  " << vector_to_string(entry.first) << ": " << entry.second << std::endl;
        }
        oss << "Column tuple ranges: " << std::endl;
        for (const auto& entry : this->column_range) {
            oss << "  " << entry.first << ": " << vector_to_string(entry.second) << std::endl;
        }
        return oss.str();
    }
    
    private:
        template<typename T>
        std::string vector_to_string(const std::vector<T>& vec) {
            std::ostringstream oss;
            oss << "[";
            for (auto it = vec.begin(); it != vec.end(); ++it) {
                if (it != vec.begin()) {
                    oss << ", ";
                }
                oss << *it;
            }
            oss << "]";
            return oss.str();
        }


};

#endif // STATISTICS_H