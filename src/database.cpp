#include "database.hpp"
#include "utils.hpp"
#include "btree.hpp"
#include "schema.hpp"
#include "sql.hpp"
#include "record.hpp"
#include <iostream>
#include <algorithm>

Database::Database(const std::string& filename) : pager(filename) {
    std::vector<char> header = pager.read_bytes(0, 100);
    page_size = Utils::parse_u16(header, 16);
}

void Database::print_db_info() {
    std::cout << "database page size: " << page_size << std::endl;
    std::vector<char> page_1_header = pager.read_bytes(100, 8);
    uint16_t table_count = BTree::parse_cell_count(page_1_header, 0);
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

void Database::scan_table(uint32_t page_num, const QueryContext& ctx, int& row_count) {
    size_t offset = (static_cast<size_t>(page_num) - 1) * page_size;
    std::vector<char> page_data = pager.read_bytes(offset, page_size);

    size_t header_offset = (page_num == 1) ? 100 : 0;

    PageType type = BTree::get_page_type(page_data, header_offset);
    uint16_t cell_count = BTree::parse_cell_count(page_data, header_offset);

    if (type == PageType::LeafTable) {
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
                std::string val;
                if (ctx.where_is_pk) {
                    val = std::to_string(row_id);
                } else {
                    val = Record::parse_column_to_string(record_payload, ctx.where_col_idx);
                }
                
                if (val != ctx.where_value) {
                    continue; 
                }
            }

            if (ctx.count_mode) {
                row_count++;
            } else {
                // Print Columns
                for (size_t i = 0; i < ctx.targets.size(); ++i) {
                    std::string val;
                    if (ctx.targets[i].is_primary_key) {
                        val = std::to_string(row_id);
                    } else {
                        val = Record::parse_column_to_string(record_payload, ctx.targets[i].index);
                    }
                    std::cout << val << (i == ctx.targets.size() - 1 ? "" : "|");
                }
                std::cout << std::endl;
            }
        }

    } else if (type == PageType::InteriorTable) {
        size_t ptr_array_start = header_offset + 12;
        auto pointers = BTree::parse_cell_pointers(page_data, ptr_array_start, cell_count);

        for (uint16_t ptr : pointers) {
            size_t cursor = ptr;
            std::vector<char> cell_slice(page_data.begin() + cursor, page_data.begin() + cursor + 4);
            uint32_t left_child = BTree::parse_interior_cell_left_child(cell_slice);
            scan_table(left_child, ctx, row_count);
        }

        uint32_t right_most = BTree::get_right_most_pointer(page_data, header_offset);
        scan_table(right_most, ctx, row_count);
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
    ctx.count_mode = false;
    
    // Check for COUNT(*)
    if (q_opt->columns.size() == 1) {
        std::string col_upper = q_opt->columns[0];
        std::transform(col_upper.begin(), col_upper.end(), col_upper.begin(), ::toupper);
        if (col_upper == "COUNT(*)") {
            ctx.count_mode = true;
        }
    }

    if (!ctx.count_mode) {
        for (const auto& col_name : q_opt->columns) {
            ColumnInfo info = Schema::get_column_info(page_1, q_opt->table, col_name);
            
            if (info.index == -1) {
                std::cerr << "Column not found: " << col_name << std::endl;
                return;
            }
            ctx.targets.push_back({info.index, info.is_primary_key});
        }
    }

    // Setup Where Column Index
    if (!q_opt->where_column.empty()) {
        ColumnInfo info = Schema::get_column_info(page_1, q_opt->table, q_opt->where_column);
        if (info.index == -1) {
            std::cerr << "Filter column not found" << std::endl;
            return;
        }
        ctx.where_col_idx = info.index;
        ctx.where_is_pk = info.is_primary_key;
    } else {
        ctx.where_col_idx = -1;
        ctx.where_is_pk = false;
    }

    int row_count = 0;
    scan_table(root_page_num, ctx, row_count);

    if (ctx.count_mode) {
        std::cout << row_count << std::endl;
    }
}