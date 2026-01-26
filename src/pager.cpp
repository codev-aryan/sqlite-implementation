#include "pager.hpp"
#include <iostream>
#include <stdexcept>

Pager::Pager(const std::string& path) : file_path(path) {
    file.open(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open database file: " + path);
    }
    file.seekg(0, std::ios::beg);
}

std::vector<char> Pager::read_bytes(size_t offset, size_t size) {
    file.seekg(offset);
    if (file.fail()) {
         throw std::runtime_error("Seek failed");
    }

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    
    if (file.gcount() != static_cast<std::streamsize>(size)) {
        throw std::runtime_error("Failed to read required bytes");
    }
    
    return buffer;
}