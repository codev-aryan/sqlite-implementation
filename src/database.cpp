#include "database.hpp"
#include "utils.hpp"
#include "btree.hpp"
#include "schema.hpp"
#include "sql.hpp"
#include <iostream>

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
    auto table_name_opt = SQL::parse_select_count(query);
    if (!table_name_opt) {
        std::cerr << "Unsupported query: " << query << std::endl;
        return;
    }
    std::string table_name = *table_name_opt;

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

    std::cout << row_count << std::endl;
}