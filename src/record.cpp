#include "record.hpp"
#include "utils.hpp"
#include <stdexcept>

std::string Record::parse_string_column(const std::vector<char>& record_payload, int target_col_idx) {
    size_t cursor = 0;
    
    // 1. Read Record Header Size
    auto [header_size, header_varint_len] = Utils::read_varint(record_payload, cursor);
    cursor += header_varint_len;

    // 2. Read Serial Types
    std::vector<int64_t> column_types;
    size_t bytes_read_in_header = header_varint_len;
    
    while (bytes_read_in_header < header_size) {
        auto [type, len] = Utils::read_varint(record_payload, cursor);
        column_types.push_back(type);
        cursor += len;
        bytes_read_in_header += len;
    }

    // 3. Locate the Target Column Body
    size_t body_cursor = header_size;
    
    for (int i = 0; i < column_types.size(); ++i) {
        int64_t type = column_types[i];
        size_t col_size = get_serial_type_size(type);

        if (i == target_col_idx) {
            // Found our column. Ensure it's a string type (odd >= 13)
            if (type >= 13 && (type % 2 == 1)) {
                return std::string(record_payload.begin() + body_cursor, 
                                   record_payload.begin() + body_cursor + col_size);
            } else {
                // Not a string or null
                return ""; 
            }
        }
        body_cursor += col_size;
    }

    return "";
}

size_t Record::get_serial_type_size(int64_t serial_type) {
    if (serial_type <= 11) {
        switch (serial_type) {
            case 0: return 0; // Null
            case 1: return 1; // 8-bit int
            case 2: return 2; // 16-bit int
            case 3: return 3; // 24-bit int
            case 4: return 4; // 32-bit int
            case 5: return 6; // 48-bit int
            case 6: return 8; // 64-bit int
            case 7: return 8; // Float
            case 8: return 0; // 0
            case 9: return 0; // 1
            case 10: case 11: return 0; // Internal
        }
    }
    // String or Blob
    // odd: (N-13)/2 -> String
    // even: (N-12)/2 -> Blob
    if (serial_type % 2 == 1) return (serial_type - 13) / 2;
    return (serial_type - 12) / 2;
}