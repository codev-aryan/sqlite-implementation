#include <iostream>
#include <string>
#include "database.hpp"

int main(int argc, char* argv[]) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc < 3) {
        std::cerr << "Expected database file and command" << std::endl;
        return 1;
    }

    std::string database_file_path = argv[1];
    std::string command = argv[2];

    try {
        Database db(database_file_path);

        if (command == ".dbinfo") {
            db.print_db_info();
        } else if (command == ".tables") {
            db.list_tables();
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}