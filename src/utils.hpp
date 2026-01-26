#pragma once
#include <cstdint>
#include <vector>
#include <stdexcept>

class Utils {
public:
    static uint16_t parse_u16(const std::vector<char>& buffer, size_t offset) {
        if (offset + 2 > buffer.size()) {
            throw std::out_of_range("Buffer overflow reading u16");
        }
        auto* bytes = reinterpret_cast<const unsigned char*>(buffer.data() + offset);
        return (static_cast<uint16_t>(bytes[0]) << 8) | bytes[1];
    }
};