#include "btree.hpp"
#include "utils.hpp"

uint16_t BTree::parse_cell_count(const std::vector<char>& header_buffer) {
    // The B-Tree page header structure (Leaf Table):
    // Byte 0: Flag
    // Byte 1-2: Free block pointer
    // Byte 3-4: Number of cells (Big-Endian)
    
    // We expect the buffer to start exactly at the page header.
    return Utils::parse_u16(header_buffer, 3);
}