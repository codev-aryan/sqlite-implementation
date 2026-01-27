#include "btree.hpp"
#include "utils.hpp"

PageType BTree::get_page_type(const std::vector<char>& page_data, size_t header_offset) {
    if (header_offset >= page_data.size()) return PageType::Unknown;
    uint8_t flag = static_cast<uint8_t>(page_data[header_offset]);
    if (flag == 0x05) return PageType::InteriorTable;
    if (flag == 0x0D) return PageType::LeafTable;
    return PageType::Unknown;
}

uint16_t BTree::parse_cell_count(const std::vector<char>& page_data, size_t header_offset) {
    return Utils::parse_u16(page_data, header_offset + 3);
}

uint32_t BTree::get_right_most_pointer(const std::vector<char>& page_data, size_t header_offset) {
    // Right-most pointer is at offset 8 in header (4 bytes)
    return Utils::parse_u32(page_data, header_offset + 8);
}

std::vector<uint16_t> BTree::parse_cell_pointers(const std::vector<char>& page_data, size_t array_start_offset, uint16_t cell_count) {
    std::vector<uint16_t> pointers;
    for (int i = 0; i < cell_count; ++i) {
        size_t ptr_offset = array_start_offset + (i * 2);
        pointers.push_back(Utils::parse_u16(page_data, ptr_offset));
    }
    return pointers;
}

uint32_t BTree::parse_interior_cell_left_child(const std::vector<char>& cell_data) {
    // Interior Table Cell: [4-byte page number] [varint key]
    return Utils::parse_u32(cell_data, 0);
}