#include <fstream>
#include <fmt/format.h>

#include "database.h"
#include "utils.h"


void Database::saveToFile(const std::string& command) {
    std::string filename = removeTrailingSemicolon(trim(command));
    if (filename.empty()) {
        throw std::runtime_error("Syntax error in SAVE command.");
    }

    std::ofstream ofs(filename);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for saving.");
    }

    for (const auto& pair : tables) {
        const Table& table = pair.second;

        ofs << "# Table: " << table.name << "\n";

        // Save column headers
        for (size_t i = 0; i < table.columns.size(); ++i) {
            ofs << table.columns[i].name;
            if (i < table.columns.size() - 1) {
                ofs << ",";
            }
        }
        ofs << "\n";

        // Save rows
        for (const auto& row : table.rows) {
            for (size_t i = 0; i < row.values.size(); ++i) {
                if (std::holds_alternative<int>(row.values[i])) {
                    ofs << std::get<int>(row.values[i]);
                } else if (std::holds_alternative<float>(row.values[i])) {
                    ofs << std::get<float>(row.values[i]);
                } else if (std::holds_alternative<char>(row.values[i])) {
                    ofs << std::get<char>(row.values[i]);
                } else if (std::holds_alternative<std::string>(row.values[i])) {
                    ofs << std::get<std::string>(row.values[i]); // No quotes
                }

                if (i < row.values.size() - 1) {
                    ofs << ",";
                }
            }
            ofs << "\n";
        }

        ofs << "\n";
    }

    ofs.close();
    fmt::print("Database saved to '{}' in CSV format successfully.\n", filename);
}


void Database::loadFromFile(const std::string& command) {
    // Expected format: "filename;"
    std::string filename = removeTrailingSemicolon(trim(command));
    if (filename.empty()) {
        throw std::runtime_error("Syntax error in LOAD command.");
    }

    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Failed to open file for loading.");
    }

    tables.clear(); // Clear current database

    std::string line;
    Table* currentTable = nullptr;

    while (std::getline(ifs, line)) {
        line = trim(line);

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Check for table name (comment line)
        if (line.starts_with("# Table:")) {
            std::string tableName = trim(line.substr(8)); // Extract table name
            Table table;
            table.name = tableName;
            tables[tableName] = table;
            currentTable = &tables[tableName];
            continue;
        }

        // If currentTable is null, we are missing a table declaration
        if (currentTable == nullptr) {
            throw std::runtime_error("Unexpected data format: No table declared.");
        }

        // Parse the first non-comment line as column headers
        if (currentTable->columns.empty()) {
            std::vector<std::string> columnHeaders = split(line, ',');
            for (const auto& header : columnHeaders) {
                Column column = {trim(header), DataType::VARCHAR}; // Default to VARCHAR
                currentTable->columns.push_back(column);
            }
            continue;
        }

        // Parse rows of data
        std::vector<std::string> rowValues = split(line, ',');
        if (rowValues.size() != currentTable->columns.size()) {
            throw std::runtime_error("Row data does not match column count in table '" + currentTable->name + "'.");
        }

        Row row;
        for (const auto& value : rowValues) {
            row.values.push_back(trim(value));
        }
        currentTable->rows.push_back(row);
    }

    ifs.close();
    fmt::print("Database loaded from '{}' in CSV format successfully.\n", filename);
}