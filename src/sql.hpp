#pragma once
#include <string>
#include <optional>

class SQL {
public:
    static std::optional<std::string> parse_select_count(const std::string& query);
};