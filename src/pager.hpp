#pragma once
#include <fstream>
#include <string>
#include <vector>

class Pager {
private:
    std::ifstream file;
    std::string file_path;

public:
    explicit Pager(const std::string& path);
    
    // Reads a specific number of bytes from an absolute offset
    std::vector<char> read_bytes(size_t offset, size_t size);
};