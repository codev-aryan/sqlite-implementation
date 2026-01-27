#include "sql.hpp"
#include <sstream>
#include <vector>
#include <algorithm>

std::optional<SelectQuery> SQL::parse_select(const std::string& query) {
    // Basic parser for "SELECT column FROM table" or "SELECT COUNT(*) FROM table"
    std::stringstream ss(query);
    std::string segment;
    std::vector<std::string> parts;
    
    while (std::getline(ss, segment, ' ')) {
        if (!segment.empty()) {
            parts.push_back(segment);
        }
    }

    if (parts.size() < 4) return std::nullopt;
    
    std::string op = parts[0];
    std::transform(op.begin(), op.end(), op.begin(), ::toupper);
    if (op != "SELECT") return std::nullopt;

    // Determine basic structure matches SELECT ... FROM ...
    // Parts: [SELECT, col, FROM, table]
    // Simple check for FROM keyword
    size_t from_index = 0;
    for(size_t i = 1; i < parts.size(); i++) {
        std::string upper = parts[i];
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        if (upper == "FROM") {
            from_index = i;
            break;
        }
    }

    if (from_index == 0 || from_index + 1 >= parts.size()) return std::nullopt;

    std::string column = parts[1]; // simplified: assumes single column, no spaces
    // Handle "COUNT(*)" vs "name"
    if (parts.size() > from_index + 1) {
        return SelectQuery{column, parts[from_index + 1]};
    }

    return std::nullopt;
}