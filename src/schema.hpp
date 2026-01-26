#pragma once
#include <vector>
#include <string>

class Schema {
public:
    // Parses Page 1 to find all table names
    static std::vector<std::string> get_table_names(const std::vector<char>& page_1_data);
};