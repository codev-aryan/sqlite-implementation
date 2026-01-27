#pragma once
#include <string>
#include <optional>

struct SelectQuery {
    std::string column;
    std::string table;
};

class SQL {
public:
    static std::optional<SelectQuery> parse_select(const std::string& query);
};