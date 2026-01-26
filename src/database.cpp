#include "database.hpp"
#include "utils.hpp"
#include <iostream>

Database::Database(const std::string& filename) : pager(filename) {}

void Database::parse_header() {
    // SQLite header is the first 100 bytes of the file
    // Page size is located at offset 16 (2 bytes)
    std::vector<char> header = pager.read_bytes(0, 100);
    
    uint16_t page_size = Utils::parse_u16(header, 16);
    std::cout << "database page size: " << page_size << std::endl;
}