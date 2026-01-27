#pragma once
#include "pager.hpp"
#include <string>
#include <vector>
#include <optional>

struct ColumnTarget {
    int index;
    bool is_primary_key;
};

struct QueryContext {
    std::vector<ColumnTarget> targets;
    int where_col_idx;
    bool where_is_pk;
    std::string where_value;
    bool count_mode;
};

class Database {
private:
    Pager pager;
    uint16_t page_size; 

    void scan_table(uint32_t page_num, const QueryContext& ctx, int& row_count);
    
    // New: Index Scan logic
    void scan_index(uint32_t page_num, uint32_t table_root_page, const QueryContext& ctx, int& row_count);
    
    // New: Fetch row by ID
    std::optional<std::vector<char>> get_row_by_id(uint32_t page_num, int64_t row_id);

public:
    explicit Database(const std::string& filename);
    void print_db_info();
    void list_tables();
    void execute_sql(const std::string& query);
};