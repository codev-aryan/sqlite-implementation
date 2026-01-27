#pragma once
#include <vector>
#include <cstdint>

enum class PageType {
    InteriorIndex = 0x02,
    InteriorTable = 0x05,
    LeafIndex = 0x0A,
    LeafTable = 0x0D,
    Unknown = 0x00
};

class BTree {
public:
    static PageType get_page_type(const std::vector<char>& page_data, size_t header_offset);
    static uint16_t parse_cell_count(const std::vector<char>& page_data, size_t header_offset);
    static uint32_t get_right_most_pointer(const std::vector<char>& page_data, size_t header_offset);
    
    // Updated: takes absolute offset to start of pointer array
    static std::vector<uint16_t> parse_cell_pointers(const std::vector<char>& page_data, size_t array_start_offset, uint16_t cell_count);
    
    // Returns the left child page number from an interior cell
    static uint32_t parse_interior_cell_left_child(const std::vector<char>& cell_data);
};