#include "sql.hpp"
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>

std::optional<SelectQuery> SQL::parse_select(const std::string& query) {
    std::string q_upper = query;
    std::transform(q_upper.begin(), q_upper.end(), q_upper.begin(), ::toupper);

    size_t select_pos = q_upper.find("SELECT");
    size_t from_pos = q_upper.find("FROM");

    if (select_pos == std::string::npos || from_pos == std::string::npos || from_pos < select_pos) {
        return std::nullopt;
    }

    size_t col_start = select_pos + 6;
    std::string cols_str = query.substr(col_start, from_pos - col_start);
    
    // Check for WHERE clause
    size_t where_pos = q_upper.find("WHERE");
    std::string table_str;
    
    std::string where_col = "";
    std::string where_val = "";

    if (where_pos != std::string::npos) {
        table_str = query.substr(from_pos + 4, where_pos - (from_pos + 4));
        
        // Parse WHERE clause: col = val
        std::string where_part = query.substr(where_pos + 5);
        size_t eq_pos = where_part.find('=');
        if (eq_pos != std::string::npos) {
            std::string w_col_raw = where_part.substr(0, eq_pos);
            std::string w_val_raw = where_part.substr(eq_pos + 1);

            // Trim column
            size_t wc_first = w_col_raw.find_first_not_of(" \t\n\r");
            size_t wc_last = w_col_raw.find_last_not_of(" \t\n\r");
            if (wc_first != std::string::npos) {
                where_col = w_col_raw.substr(wc_first, (wc_last - wc_first + 1));
            }

            // Trim value and handle quotes
            size_t wv_first = w_val_raw.find_first_not_of(" \t\n\r");
            size_t wv_last = w_val_raw.find_last_not_of(" \t\n\r");
            if (wv_first != std::string::npos) {
                where_val = w_val_raw.substr(wv_first, (wv_last - wv_first + 1));
                if (where_val.size() >= 2 && (where_val.front() == '\'' || where_val.front() == '"')) {
                    where_val = where_val.substr(1, where_val.size() - 2);
                }
            }
        }
    } else {
        table_str = query.substr(from_pos + 4);
    }

    size_t first = table_str.find_first_not_of(" \t\n\r");
    size_t last = table_str.find_last_not_of(" \t\n\r");
    if (first == std::string::npos) return std::nullopt;
    std::string table_name = table_str.substr(first, (last - first + 1));

    std::vector<std::string> columns;
    std::stringstream ss(cols_str);
    std::string segment;
    while (std::getline(ss, segment, ',')) {
        size_t c_first = segment.find_first_not_of(" \t\n\r");
        size_t c_last = segment.find_last_not_of(" \t\n\r");
        if (c_first != std::string::npos) {
            columns.push_back(segment.substr(c_first, (c_last - c_first + 1)));
        }
    }

    if (columns.empty()) return std::nullopt;

    return SelectQuery{columns, table_name, where_col, where_val};
}