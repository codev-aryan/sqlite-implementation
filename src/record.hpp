#pragma once
#include <vector>
#include <string>
#include <cstdint>

class Record {
public:
    static std::string parse_string_column(const std::vector<char>& record_payload, int target_col_idx);
    static int64_t parse_int_column(const std::vector<char>& record_payload, int target_col_idx);

private:
    static size_t get_serial_type_size(int64_t serial_type);
    static int64_t read_big_endian_int(const std::vector<char>& buffer, size_t offset, size_t size);
};