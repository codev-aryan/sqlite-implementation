#include "database.hpp"
#include "utils.hpp"
#include "btree.hpp"
#include "schema.hpp"
#include "sql.hpp"
#include "record.hpp"
#include <iostream>
#include <algorithm>

Database::Database(const std::string& filename) : pager(filename) {}

void Database::print_db_info() {
    std::vector<char> file_header = pager.read_bytes(0, 100);
    uint16_t page_size = Utils::parse_u16(file_header, 16);
    std::cout << "database page size: " << page_size << std::endl;

    std::vector<char> page_1_header = pager.read_bytes(100, 8);
    uint16_t table_count = BTree::parse_cell_count(page_1_header);
    std::cout << "number of tables: " << table_count << std::endl;
}

void Database::list_tables() {
    std::vector<char> header = pager.read_bytes(0, 100);
    uint16_t page_size = Utils::parse_u16(header, 16);
    std::vector<char> page_1 = pager.read_bytes(0, page_size);
    auto tables = Schema::get_table_names(page_1);
    for (size_t i = 0; i < tables.size(); ++i) {
        std::cout << tables[i] << (i == tables.size() - 1 ? "" : " ");
    }
    std::cout << std::endl;
}

void Database::execute_sql(const std::string& query) {
    auto q_opt = SQL::parse_select(query);
    if (!q_opt) {
        std::cerr << "Unsupported query: " << query << std::endl;
        return;
    }
    std::string table_name = q_opt->table;
    std::vector<std::string> columns = q_opt->columns;

    std::vector<char> header = pager.read_bytes(0, 100);
    uint16_t page_size = Utils::parse_u16(header, 16);
    std::vector<char> page_1 = pager.read_bytes(0, page_size);
    
    int root_page_num = Schema::get_root_page_number(page_1, table_name);
    if (root_page_num == -1) {
        std::cerr << "Table not found: " << table_name << std::endl;
        return;
    }

    size_t offset = (static_cast<size_t>(root_page_num) - 1) * page_size;
    std::vector<char> root_page = pager.read_bytes(offset, page_size);
    std::vector<char> page_header(root_page.begin(), root_page.begin() + 8);
    uint16_t row_count = BTree::parse_cell_count(page_header);

    // Check for COUNT(*)
    if (columns.size() == 1) {
        std::string col_upper = columns[0];
        std::transform(col_upper.begin(), col_upper.end(), col_upper.begin(), ::toupper);
        if (col_upper == "COUNT(*)") {
            std::cout << row_count << std::endl;
            return;
        }
    }

    // Resolve Column Indices
    std::vector<int> col_indices;
    for (const auto& col_name : columns) {
        int idx = Schema::get_column_index(page_1, table_name, col_name);
        if (idx == -1) {
            std::cerr << "Column not found: " << col_name << std::endl;
            return;
        }
        col_indices.push_back(idx);
    }

    // Iterate Rows
    auto pointers = BTree::parse_cell_pointers(root_page, 0, row_count);
    for (uint16_t ptr : pointers) {
        size_t cursor = ptr;
        auto [payload_size, s1] = Utils::read_varint(root_page, cursor);
        cursor += s1;
        auto [row_id, s2] = Utils::read_varint(root_page, cursor);
        cursor += s2;
        
        std::vector<char> record_payload(root_page.begin() + cursor, 
                                         root_page.begin() + cursor + payload_size);
        
        for (size_t i = 0; i < col_indices.size(); ++i) {
            std::string val = Record::parse_column_to_string(record_payload, col_indices[i]);
            std::cout << val << (i == col_indices.size() - 1 ? "" : "|");
        }
        std::cout << std::endl;
    }
}