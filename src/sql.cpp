#include "sql.hpp"
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>

std::optional<SelectQuery> SQL::parse_select(const std::string& query) {
    // robust parsing for "SELECT col1, col2, col3 FROM table"
    std::string q_upper = query;
    std::transform(q_upper.begin(), q_upper.end(), q_upper.begin(), ::toupper);

    size_t select_pos = q_upper.find("SELECT");
    size_t from_pos = q_upper.find("FROM");

    if (select_pos == std::string::npos || from_pos == std::string::npos || from_pos < select_pos) {
        return std::nullopt;
    }

    // Extract columns part
    size_t col_start = select_pos + 6;
    std::string cols_str = query.substr(col_start, from_pos - col_start);
    
    // Extract table part
    std::string table_str = query.substr(from_pos + 4);
    
    // Trim table name
    size_t first = table_str.find_first_not_of(" \t\n\r");
    size_t last = table_str.find_last_not_of(" \t\n\r");
    if (first == std::string::npos) return std::nullopt;
    std::string table_name = table_str.substr(first, (last - first + 1));

    // Parse columns
    std::vector<std::string> columns;
    std::stringstream ss(cols_str);
    std::string segment;
    
    while (std::getline(ss, segment, ',')) {
        // Trim whitespace
        size_t c_first = segment.find_first_not_of(" \t\n\r");
        size_t c_last = segment.find_last_not_of(" \t\n\r");
        if (c_first != std::string::npos) {
            columns.push_back(segment.substr(c_first, (c_last - c_first + 1)));
        }
    }

    if (columns.empty()) return std::nullopt;

    return SelectQuery{columns, table_name};
}