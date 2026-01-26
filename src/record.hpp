#pragma once
#include <vector>
#include <string>
#include <cstdint>

class Record {
public:
    // Parses a record and returns the string value of the specified column index
    static std::string parse_string_column(const std::vector<char>& record_payload, int target_col_idx);

private:
    static size_t get_serial_type_size(int64_t serial_type);
};