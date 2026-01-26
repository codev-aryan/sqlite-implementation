#include "database.hpp"
#include "utils.hpp"
#include "btree.hpp"
#include <iostream>

Database::Database(const std::string& filename) : pager(filename) {}

void Database::print_db_info() {
    // 1. Read Page Size (first 2 bytes at offset 16 in the file header)
    // The file header is the first 100 bytes of Page 1.
    std::vector<char> file_header = pager.read_bytes(0, 100);
    uint16_t page_size = Utils::parse_u16(file_header, 16);
    std::cout << "database page size: " << page_size << std::endl;

    // 2. Read Number of Tables (Cell count of Page 1)
    // Page 1 is special: The B-Tree page header starts AFTER the 100-byte file header.
    // So the B-Tree header starts at offset 100.
    // We need the first few bytes of that header to get the cell count.
    // Standard B-Tree header is 8 or 12 bytes. We safely read 8 bytes.
    std::vector<char> page_1_header = pager.read_bytes(100, 8);

    uint16_t table_count = BTree::parse_cell_count(page_1_header);
    std::cout << "number of tables: " << table_count << std::endl;
}