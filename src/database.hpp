#pragma once
#include "pager.hpp"
#include <string>
#include <vector>

struct QueryContext {
    std::vector<int> col_indices;
    int where_col_idx;
    std::string where_value;
};

class Database {
private:
    Pager pager;
    uint16_t page_size; // Cache page size

    // Recursive function to traverse B-Tree
    void scan_table(uint32_t page_num, const QueryContext& ctx);

public:
    explicit Database(const std::string& filename);
    void print_db_info();
    void list_tables();
    void execute_sql(const std::string& query);
};