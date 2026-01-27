#include "record.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <algorithm>

size_t Record::get_serial_type_size(int64_t serial_type) {
    if (serial_type <= 11) {
        switch (serial_type) {
            case 0: return 0;
            case 1: return 1;
            case 2: return 2;
            case 3: return 3;
            case 4: return 4;
            case 5: return 6;
            case 6: return 8;
            case 7: return 8;
            case 8: return 0;
            case 9: return 0;
            case 10: case 11: return 0;
        }
    }
    if (serial_type % 2 == 1) return (serial_type - 13) / 2;
    return (serial_type - 12) / 2;
}

int64_t Record::read_big_endian_int(const std::vector<char>& buffer, size_t offset, size_t size) {
    int64_t value = 0;
    for (size_t i = 0; i < size; ++i) {
        // Sign extension logic omitted for brevity as generally positive rowids/values used here
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    return value;
}

std::string Record::parse_column_to_string(const std::vector<char>& record_payload, int target_col_idx) {
    size_t cursor = 0;
    auto [header_size, header_varint_len] = Utils::read_varint(record_payload, cursor);
    cursor += header_varint_len;

    std::vector<int64_t> column_types;
    size_t bytes_read_in_header = header_varint_len;
    
    while (bytes_read_in_header < header_size) {
        auto [type, len] = Utils::read_varint(record_payload, cursor);
        column_types.push_back(type);
        cursor += len;
        bytes_read_in_header += len;
    }

    size_t body_cursor = header_size;
    
    for (int i = 0; i < column_types.size(); ++i) {
        int64_t type = column_types[i];
        size_t col_size = get_serial_type_size(type);

        if (i == target_col_idx) {
            // Integer types
            if (type >= 1 && type <= 6) {
                int64_t val = read_big_endian_int(record_payload, body_cursor, col_size);
                return std::to_string(val);
            }
            if (type == 8) return "0";
            if (type == 9) return "1";
            
            // String types
            if (type >= 13 && (type % 2 == 1)) {
                return std::string(record_payload.begin() + body_cursor, 
                                   record_payload.begin() + body_cursor + col_size);
            }
            
            // Null or Blob (returning empty for now)
            return "";
        }
        body_cursor += col_size;
    }
    return "";
}

std::string Record::parse_string_column(const std::vector<char>& record_payload, int target_col_idx) {
    return parse_column_to_string(record_payload, target_col_idx);
}

int64_t Record::parse_int_column(const std::vector<char>& record_payload, int target_col_idx) {
    size_t cursor = 0;
    auto [header_size, header_varint_len] = Utils::read_varint(record_payload, cursor);
    cursor += header_varint_len;

    std::vector<int64_t> column_types;
    size_t bytes_read_in_header = header_varint_len;
    while (bytes_read_in_header < header_size) {
        auto [type, len] = Utils::read_varint(record_payload, cursor);
        column_types.push_back(type);
        cursor += len;
        bytes_read_in_header += len;
    }
    size_t body_cursor = header_size;
    for (int i = 0; i < column_types.size(); ++i) {
        int64_t type = column_types[i];
        size_t col_size = get_serial_type_size(type);
        if (i == target_col_idx) {
            if (type == 8) return 0;
            if (type == 9) return 1;
            if (type >= 1 && type <= 6) return read_big_endian_int(record_payload, body_cursor, col_size);
            return -1; 
        }
        body_cursor += col_size;
    }
    return -1;
}