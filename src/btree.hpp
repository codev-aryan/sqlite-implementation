#pragma once
#include <vector>
#include <cstdint>

class BTree {
public:
    // Reads the cell count from a B-Tree page header buffer.
    // The buffer should start at the beginning of the Page Header.
    static uint16_t parse_cell_count(const std::vector<char>& header_buffer);
};