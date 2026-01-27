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
    
    // Updated: Returns struct with index and primary key status
    static ColumnInfo get_column_info(const std::vector<char>& page_1_data, const std::string& table_name, const std::string& column_name);
    
    // Keep legacy for backward compatibility if needed, or just update callers
    static int get_column_index(const std::vector<char>& page_1_data, const std::string& table_name, const std::string& column_name);
};