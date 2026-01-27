#pragma once
#include <string>
#include <vector>
#include <optional>

struct SelectQuery {
    std::vector<std::string> columns;
    std::string table;
};

class SQL {
public:
    static std::optional<SelectQuery> parse_select(const std::string& query);
};