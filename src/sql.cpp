#include "sql.hpp"
#include <sstream>
#include <vector>
#include <algorithm>

std::optional<std::string> SQL::parse_select_count(const std::string& query) {
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

    return parts.back();
}