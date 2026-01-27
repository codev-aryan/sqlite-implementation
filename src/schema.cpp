#include "schema.hpp"
#include "btree.hpp"
#include "record.hpp"
#include "utils.hpp"

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