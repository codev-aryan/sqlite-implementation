#pragma once
#include <vector>
#include <string>

struct ColumnInfo {
    int index;
    bool is_primary_key;
};

class Schema {
public:
    static std::vector<std::string> get_table_names(const std::vector<char>& page_1_data);
    static int get_root_page_number(const std::vector<char>& page_1_data, const std::string& target_table);
    static ColumnInfo get_column_info(const std::vector<char>& page_1_data, const std::string& table_name, const std::string& column_name);
    static int get_column_index(const std::vector<char>& page_1_data, const std::string& table_name, const std::string& column_name);
    
    // New: Find root page of an index by name
    static int get_index_root_page(const std::vector<char>& page_1_data, const std::string& index_name);
};