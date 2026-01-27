#pragma once
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <utility>

class Utils {
public:
    static uint16_t parse_u16(const std::vector<char>& buffer, size_t offset) {
        if (offset + 2 > buffer.size()) {
            throw std::out_of_range("Buffer overflow reading u16");
        }
        auto* bytes = reinterpret_cast<const unsigned char*>(buffer.data() + offset);
        return (static_cast<uint16_t>(bytes[0]) << 8) | bytes[1];
    }

    static uint32_t parse_u32(const std::vector<char>& buffer, size_t offset) {
        if (offset + 4 > buffer.size()) {
            throw std::out_of_range("Buffer overflow reading u32");
        }
        auto* bytes = reinterpret_cast<const unsigned char*>(buffer.data() + offset);
        return (static_cast<uint32_t>(bytes[0]) << 24) | 
               (static_cast<uint32_t>(bytes[1]) << 16) | 
               (static_cast<uint32_t>(bytes[2]) << 8) | 
               bytes[3];
    }

    static std::pair<uint64_t, int> read_varint(const std::vector<char>& buffer, size_t offset) {
        uint64_t value = 0;
        int bytes_read = 0;
        
        for (int i = 0; i < 9; ++i) {
            if (offset + i >= buffer.size()) break;
            uint8_t byte = static_cast<uint8_t>(buffer[offset + i]);
            bytes_read++;
            
            if (i < 8) {
                value = (value << 7) | (byte & 0x7F);
                if ((byte & 0x80) == 0) break;
            } else {
                value = (value << 8) | byte;
            }
        }
        return {value, bytes_read};
    }
};