#pragma once
#include <vector>
#include <cstdint>

class BTree {
public:
    // Reads the cell count from a B-Tree page header buffer.
    static uint16_t parse_cell_count(const std::vector<char>& header_buffer);
    
    // Returns a list of absolute offsets for every cell on the page.
    static std::vector<uint16_t> parse_cell_pointers(const std::vector<char>& page_data, size_t header_offset, uint16_t cell_count);
};