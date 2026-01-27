#pragma once
#include <vector>
#include <string>

class Schema {
public:
    static std::vector<std::string> get_table_names(const std::vector<char>& page_1_data);
    static int get_root_page_number(const std::vector<char>& page_1_data, const std::string& target_table);
};