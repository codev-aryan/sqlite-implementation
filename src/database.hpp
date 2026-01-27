#pragma once
#include "pager.hpp"
#include <string>
#include <vector>

struct ColumnTarget {
    int index;
    bool is_primary_key;
};

struct QueryContext {
    std::vector<ColumnTarget> targets;
    int where_col_idx;
    bool where_is_pk;
    std::string where_value;
    bool count_mode; // New flag for COUNT(*)
};

class Database {
private:
    Pager pager;
    uint16_t page_size; 

    // Updated: accepts row_count reference
    void scan_table(uint32_t page_num, const QueryContext& ctx, int& row_count);

public:
    explicit Database(const std::string& filename);
    void print_db_info();
    void list_tables();
    void execute_sql(const std::string& query);
};