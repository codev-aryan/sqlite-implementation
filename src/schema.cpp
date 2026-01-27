#include "schema.hpp"
#include "btree.hpp"
#include "record.hpp"
#include "utils.hpp"
#include <sstream>
#include <algorithm>

std::vector<std::string> Schema::get_table_names(const std::vector<char>& page_1_data) {
    std::vector<std::string> tables;
    size_t page_header_offset = 100;
    std::vector<char> header_bytes(page_1_data.begin() + page_header_offset, 
                                   page_1_data.begin() + page_header_offset + 8);
    uint16_t cell_count = BTree::parse_cell_count(header_bytes);
    auto cell_pointers = BTree::parse_cell_pointers(page_1_data, page_header_offset, cell_count);
    
    for (uint16_t offset : cell_pointers) {
        size_t cursor = offset;
        auto [payload_size, s1] = Utils::read_varint(page_1_data, cursor);
        cursor += s1;
        auto [row_id, s2] = Utils::read_varint(page_1_data, cursor);
        cursor += s2;
        std::vector<char> record_payload(page_1_data.begin() + cursor, 
                                         page_1_data.begin() + cursor + payload_size);
        std::string tbl_name = Record::parse_string_column(record_payload, 2);
        std::string type = Record::parse_string_column(record_payload, 0);
        if (type == "table" && tbl_name != "sqlite_sequence") {
            tables.push_back(tbl_name);
        }
    }
    return tables;
}

int Schema::get_root_page_number(const std::vector<char>& page_1_data, const std::string& target_table) {
    size_t page_header_offset = 100;
    std::vector<char> header_bytes(page_1_data.begin() + page_header_offset, 
                                   page_1_data.begin() + page_header_offset + 8);
    uint16_t cell_count = BTree::parse_cell_count(header_bytes);
    auto cell_pointers = BTree::parse_cell_pointers(page_1_data, page_header_offset, cell_count);
    
    for (uint16_t offset : cell_pointers) {
        size_t cursor = offset;
        auto [payload_size, s1] = Utils::read_varint(page_1_data, cursor);
        cursor += s1;
        auto [row_id, s2] = Utils::read_varint(page_1_data, cursor);
        cursor += s2;
        std::vector<char> record_payload(page_1_data.begin() + cursor, 
                                         page_1_data.begin() + cursor + payload_size);
        std::string tbl_name = Record::parse_string_column(record_payload, 2);
        if (tbl_name == target_table) {
            return static_cast<int>(Record::parse_int_column(record_payload, 3));
        }
    }
    return -1;
}

int Schema::get_column_index(const std::vector<char>& page_1_data, const std::string& table_name, const std::string& column_name) {
    // 1. Find the SQL for the table (column index 4 in sqlite_schema)
    std::string create_sql = "";
    
    size_t page_header_offset = 100;
    std::vector<char> header_bytes(page_1_data.begin() + page_header_offset, 
                                   page_1_data.begin() + page_header_offset + 8);
    uint16_t cell_count = BTree::parse_cell_count(header_bytes);
    auto cell_pointers = BTree::parse_cell_pointers(page_1_data, page_header_offset, cell_count);
    
    for (uint16_t offset : cell_pointers) {
        size_t cursor = offset;
        auto [payload_size, s1] = Utils::read_varint(page_1_data, cursor);
        cursor += s1;
        auto [row_id, s2] = Utils::read_varint(page_1_data, cursor);
        cursor += s2;
        std::vector<char> record_payload(page_1_data.begin() + cursor, 
                                         page_1_data.begin() + cursor + payload_size);
        std::string tbl_name = Record::parse_string_column(record_payload, 2);
        if (tbl_name == table_name) {
            create_sql = Record::parse_string_column(record_payload, 4);
            break;
        }
    }
    
    if (create_sql.empty()) return -1;

    // 2. Parse "CREATE TABLE name (col1 type, col2 type...)"
    // Find content between first ( and last )
    size_t start = create_sql.find('(');
    size_t end = create_sql.rfind(')');
    if (start == std::string::npos || end == std::string::npos) return -1;
    
    std::string columns_def = create_sql.substr(start + 1, end - start - 1);
    
    // Split by comma
    std::stringstream ss(columns_def);
    std::string segment;
    int index = 0;
    
    // Normalize target name (remove quotes if any)
    std::string target = column_name;
    
    while (std::getline(ss, segment, ',')) {
        // Trim whitespace
        size_t first = segment.find_first_not_of(" \t\n\r");
        size_t last = segment.find_last_not_of(" \t\n\r");
        if (first == std::string::npos) continue;
        std::string col_def = segment.substr(first, (last - first + 1));
        
        // First word is column name
        std::string col_name = col_def.substr(0, col_def.find_first_of(" \t"));
        
        // Remove quotes if present
        if (col_name.size() >= 2 && (col_name.front() == '"' || col_name.front() == '`' || col_name.front() == '\'')) {
             col_name = col_name.substr(1, col_name.size() - 2);
        }

        if (col_name == target) {
            return index;
        }
        index++;
    }

    return -1;
}