#include "schema.hpp"
#include "btree.hpp"
#include "record.hpp"
#include "utils.hpp"
#include <sstream>
#include <algorithm>

static size_t page_1_ptr_start = 100 + 8;

std::vector<std::string> Schema::get_table_names(const std::vector<char>& page_1_data) {
    std::vector<std::string> tables;
    size_t page_header_offset = 100;
    uint16_t cell_count = BTree::parse_cell_count(page_1_data, page_header_offset);
    auto cell_pointers = BTree::parse_cell_pointers(page_1_data, page_1_ptr_start, cell_count);
    
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
    uint16_t cell_count = BTree::parse_cell_count(page_1_data, page_header_offset);
    auto cell_pointers = BTree::parse_cell_pointers(page_1_data, page_1_ptr_start, cell_count);
    
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

ColumnInfo Schema::get_column_info(const std::vector<char>& page_1_data, const std::string& table_name, const std::string& column_name) {
    std::string create_sql = "";
    size_t page_header_offset = 100;
    uint16_t cell_count = BTree::parse_cell_count(page_1_data, page_header_offset);
    auto cell_pointers = BTree::parse_cell_pointers(page_1_data, page_1_ptr_start, cell_count);
    
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
    
    if (create_sql.empty()) return {-1, false};

    size_t start = create_sql.find('(');
    size_t end = create_sql.rfind(')');
    if (start == std::string::npos || end == std::string::npos) return {-1, false};
    
    std::string columns_def = create_sql.substr(start + 1, end - start - 1);
    std::stringstream ss(columns_def);
    std::string segment;
    int index = 0;
    std::string target = column_name;
    
    while (std::getline(ss, segment, ',')) {
        size_t first = segment.find_first_not_of(" \t\n\r");
        size_t last = segment.find_last_not_of(" \t\n\r");
        if (first == std::string::npos) continue;
        std::string col_def = segment.substr(first, (last - first + 1));
        std::string col_name = col_def.substr(0, col_def.find_first_of(" \t"));
        if (col_name.size() >= 2 && (col_name.front() == '"' || col_name.front() == '`' || col_name.front() == '\'')) {
             col_name = col_name.substr(1, col_name.size() - 2);
        }
        if (col_name == target) {
            std::string def_upper = col_def;
            std::transform(def_upper.begin(), def_upper.end(), def_upper.begin(), ::toupper);
            bool is_pk = def_upper.find("PRIMARY KEY") != std::string::npos;
            return {index, is_pk};
        }
        index++;
    }
    return {-1, false};
}

int Schema::get_column_index(const std::vector<char>& page_1_data, const std::string& table_name, const std::string& column_name) {
    return get_column_info(page_1_data, table_name, column_name).index;
}

int Schema::get_index_root_page(const std::vector<char>& page_1_data, const std::string& index_name) {
    size_t page_header_offset = 100;
    uint16_t cell_count = BTree::parse_cell_count(page_1_data, page_header_offset);
    auto cell_pointers = BTree::parse_cell_pointers(page_1_data, page_1_ptr_start, cell_count);
    
    for (uint16_t offset : cell_pointers) {
        size_t cursor = offset;
        auto [payload_size, s1] = Utils::read_varint(page_1_data, cursor);
        cursor += s1;
        auto [row_id, s2] = Utils::read_varint(page_1_data, cursor);
        cursor += s2;
        std::vector<char> record_payload(page_1_data.begin() + cursor, 
                                         page_1_data.begin() + cursor + payload_size);
        
        // sqlite_schema: type(0), name(1), tbl_name(2), rootpage(3), sql(4)
        std::string name = Record::parse_string_column(record_payload, 1);
        std::string type = Record::parse_string_column(record_payload, 0);
        
        if (type == "index" && name == index_name) {
             return static_cast<int>(Record::parse_int_column(record_payload, 3)); // rootpage
        }
    }
    return -1;
}