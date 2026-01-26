#pragma once
#include "pager.hpp"
#include <string>

class Database {
private:
    Pager pager;

public:
    explicit Database(const std::string& filename);
    void print_db_info();
};