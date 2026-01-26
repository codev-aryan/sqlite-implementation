#include "btree.hpp"
#include "utils.hpp"

uint16_t BTree::parse_cell_count(const std::vector<char>& header_buffer) {
    // Byte 3-4: Number of cells (Big-Endian)
    return Utils::parse_u16(header_buffer, 3);
}

std::vector<uint16_t> BTree::parse_cell_pointers(const std::vector<char>& page_data, size_t header_offset, uint16_t cell_count) {
    std::vector<uint16_t> pointers;
    // The cell pointer array follows immediately after the 8-byte (leaf) or 12-byte (interior) header.
    // For this stage (leaf), header is 8 bytes.
    // Pointers start at header_offset + 8.
    size_t array_start = header_offset + 8;
    
    for (int i = 0; i < cell_count; ++i) {
        size_t ptr_offset = array_start + (i * 2);
        pointers.push_back(Utils::parse_u16(page_data, ptr_offset));
    }
    return pointers;
}