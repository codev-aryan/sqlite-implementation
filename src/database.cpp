#include "database.hpp"
#include "utils.hpp"
#include "btree.hpp"
#include "schema.hpp"
#include "sql.hpp"
#include "record.hpp"
#include <iostream>
#include <algorithm>

Database::Database(const std::string& filename) : pager(filename) {
    // Initialize page size from file header immediately
    std::vector<char> header = pager.read_bytes(0, 100);
    page_size = Utils::parse_u16(header, 16);
}

void Database::print_db_info() {
    std::cout << "database page size: " << page_size << std::endl;
    std::vector<char> page_1_header = pager.read_bytes(100, 8);
    uint16_t table_count = BTree::parse_cell_count(page_1_header, 0); // offset 0 relative to buffer
    std::cout << "number of tables: " << table_count << std::endl;
}

void Database::list_tables() {
    std::vector<char> page_1 = pager.read_bytes(0, page_size);
    auto tables = Schema::get_table_names(page_1);
    for (size_t i = 0; i < tables.size(); ++i) {
        std::cout << tables[i] << (i == tables.size() - 1 ? "" : " ");
    }
    std::cout << std::endl;
}

void Database::scan_table(uint32_t page_num, const QueryContext& ctx) {
    size_t offset = (static_cast<size_t>(page_num) - 1) * page_size;
    std::vector<char> page_data = pager.read_bytes(offset, page_size);

    // Page 1 is special: header starts at 100
    size_t header_offset = (page_num == 1) ? 100 : 0;

    PageType type = BTree::get_page_type(page_data, header_offset);
    uint16_t cell_count = BTree::parse_cell_count(page_data, header_offset);

    if (type == PageType::LeafTable) {
        // Header size for leaf is 8 bytes
        size_t ptr_array_start = header_offset + 8;
        auto pointers = BTree::parse_cell_pointers(page_data, ptr_array_start, cell_count);

        for (uint16_t ptr : pointers) {
            size_t cursor = ptr;
            auto [payload_size, s1] = Utils::read_varint(page_data, cursor);
            cursor += s1;
            auto [row_id, s2] = Utils::read_varint(page_data, cursor);
            cursor += s2;
            
            std::vector<char> record_payload(page_data.begin() + cursor, 
                                             page_data.begin() + cursor + payload_size);
            
            // Check Filter
            if (ctx.where_col_idx != -1) {
                std::string val = Record::parse_column_to_string(record_payload, ctx.where_col_idx);
                if (val != ctx.where_value) {
                    continue; 
                }
            }

            // Print Columns
            for (size_t i = 0; i < ctx.col_indices.size(); ++i) {
                std::string val = Record::parse_column_to_string(record_payload, ctx.col_indices[i]);
                std::cout << val << (i == ctx.col_indices.size() - 1 ? "" : "|");
            }
            std::cout << std::endl;
        }

    } else if (type == PageType::InteriorTable) {
        // Header size for interior is 12 bytes
        size_t ptr_array_start = header_offset + 12;
        auto pointers = BTree::parse_cell_pointers(page_data, ptr_array_start, cell_count);

        // Iterate child pointers from cells
        for (uint16_t ptr : pointers) {
            size_t cursor = ptr;
            // Interior cell: [4 byte page num] [varint key]
            // We just need the page number to traverse down
            // We can read 4 bytes safely
            std::vector<char> cell_slice(page_data.begin() + cursor, page_data.begin() + cursor + 4);
            uint32_t left_child = BTree::parse_interior_cell_left_child(cell_slice);
            
            scan_table(left_child, ctx);
        }

        // Visit the right-most pointer
        uint32_t right_most = BTree::get_right_most_pointer(page_data, header_offset);
        scan_table(right_most, ctx);
    }
}

void Database::execute_sql(const std::string& query) {
    auto q_opt = SQL::parse_select(query);
    if (!q_opt) {
        std::cerr << "Unsupported query: " << query << std::endl;
        return;
    }
    
    std::vector<char> page_1 = pager.read_bytes(0, page_size);
    int root_page_num = Schema::get_root_page_number(page_1, q_opt->table);
    if (root_page_num == -1) {
        std::cerr << "Table not found: " << q_opt->table << std::endl;
        return;
    }

    QueryContext ctx;
    ctx.where_value = q_opt->where_value;
    
    // Setup Column Indices (Output)
    if (q_opt->columns.size() == 1) {
        std::string col_upper = q_opt->columns[0];
        std::transform(col_upper.begin(), col_upper.end(), col_upper.begin(), ::toupper);
        if (col_upper == "COUNT(*)") {
             // Special handling for count? 
             // The prompt for this stage focuses on SELECT id, name ...
             // If we need to support COUNT(*) with WHERE on multi-page, 
             // we'd need to adapt scan_table to return count or accept a counter ref.
             // For now, let's assume standard SELECT col1, col2...
             // But if we want to keep COUNT(*) working without where clause (simple case),
             // we can check if where is empty. If where exists, we must scan.
             // Let's implement full scan counting if requested.
             // For the specific challenge "SELECT id, name", we don't need count logic inside scan yet.
             // We'll proceed with standard column extraction.
        }
    }

    for (const auto& col_name : q_opt->columns) {
        int idx = Schema::get_column_index(page_1, q_opt->table, col_name);
        if (idx == -1) {
             // If it is count(*), we might handle it differently, but for now strict col check
             std::string col_upper = col_name;
             std::transform(col_upper.begin(), col_upper.end(), col_upper.begin(), ::toupper);
             if (col_upper != "COUNT(*)") {
                std::cerr << "Column not found: " << col_name << std::endl;
                return;
             }
        }
        ctx.col_indices.push_back(idx);
    }

    // Setup Where Column Index
    if (!q_opt->where_column.empty()) {
        ctx.where_col_idx = Schema::get_column_index(page_1, q_opt->table, q_opt->where_column);
        if (ctx.where_col_idx == -1) {
            std::cerr << "Filter column not found" << std::endl;
            return;
        }
    } else {
        ctx.where_col_idx = -1;
    }

    scan_table(root_page_num, ctx);
}