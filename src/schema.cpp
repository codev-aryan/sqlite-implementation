#include "schema.hpp"
#include "btree.hpp"
#include "record.hpp"
#include "utils.hpp"

std::vector<std::string> Schema::get_table_names(const std::vector<char>& page_1_data) {
    std::vector<std::string> tables;
    
    // Page 1 Header starts at 100
    size_t page_header_offset = 100;
    
    // Extract header buffer (8 bytes is enough for leaf header)
    std::vector<char> header_bytes(page_1_data.begin() + page_header_offset, 
                                   page_1_data.begin() + page_header_offset + 8);
                                   
    uint16_t cell_count = BTree::parse_cell_count(header_bytes);
    
    // Get all cell locations
    auto cell_pointers = BTree::parse_cell_pointers(page_1_data, page_header_offset, cell_count);
    
    for (uint16_t offset : cell_pointers) {
        size_t cursor = offset;
        
        // Leaf Table Cell Format:
        // 1. Payload Size (Varint)
        auto [payload_size, s1] = Utils::read_varint(page_1_data, cursor);
        cursor += s1;
        
        // 2. Row ID (Varint)
        auto [row_id, s2] = Utils::read_varint(page_1_data, cursor);
        cursor += s2;
        
        // 3. Payload (Record)
        std::vector<char> record_payload(page_1_data.begin() + cursor, 
                                         page_1_data.begin() + cursor + payload_size);
                                         
        // sqlite_schema: type(0), name(1), tbl_name(2), rootpage(3), sql(4)
        std::string tbl_name = Record::parse_string_column(record_payload, 2);
        std::string type = Record::parse_string_column(record_payload, 0);

        // Filter: only "table" type and exclude "sqlite_sequence" if strictly following user tables
        if (type == "table" && tbl_name != "sqlite_sequence") {
            tables.push_back(tbl_name);
        }
    }
    
    return tables;
}