#include "database.hpp"
#include "utils.hpp"
#include "btree.hpp"
#include "schema.hpp"
#include <iostream>

Database::Database(const std::string& filename) : pager(filename) {}

void Database::print_db_info() {
    std::vector<char> file_header = pager.read_bytes(0, 100);
    uint16_t page_size = Utils::parse_u16(file_header, 16);
    std::cout << "database page size: " << page_size << std::endl;

    std::vector<char> page_1_header = pager.read_bytes(100, 8);
    uint16_t table_count = BTree::parse_cell_count(page_1_header);
    std::cout << "number of tables: " << table_count << std::endl;
}

void Database::list_tables() {
    // Read the entire Page 1. 
    // Note: We need the page size to know how much to read, or we can just read 4096 (standard)
    // For robustness, read the header first to get page size.
    
    std::vector<char> header = pager.read_bytes(0, 100);
    uint16_t page_size = Utils::parse_u16(header, 16);
    
    std::vector<char> page_1 = pager.read_bytes(0, page_size);
    
    auto tables = Schema::get_table_names(page_1);
    
    for (size_t i = 0; i < tables.size(); ++i) {
        std::cout << tables[i] << (i == tables.size() - 1 ? "" : " ");
    }
    std::cout << std::endl;
}