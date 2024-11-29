#include <fstream>
#include <iostream> // For error messages
#include <cstdlib>  // For std::rand()

#include <filesystem>

#include "database.h"
#include "utils.h"

void Database::saveToFile(const std::string& command) {
    // Split the command on " AS " (case-sensitive match)
    std::string command_pr = removeTrailingSemicolon(trim(command));
    size_t asPos = command_pr.find(" AS ");
    std::string tableName, csvFileName;

    if (asPos != std::string::npos) {
        // "AS" is present: Extract table name and CSV file name
        tableName = trim(command_pr.substr(0, asPos));
        csvFileName = trim(command_pr.substr(asPos + 4)); // Skip " AS "
    } else {
        // "AS" not present: Treat the table name as the CSV file name
        tableName = trim(command_pr);
        csvFileName = tableName + ".csv";
    }

    if (tableName.empty() || csvFileName.empty()) {
        throw std::runtime_error("Syntax error in SAVE command. Table name or CSV file name is missing.");
    }

    // Check if the table exists in memory
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' does not exist in memory.");
    }

    // Construct the file path: Go up one directory from "cmake-build-debug" and into "data"
    const std::string filepath = "../data/" + csvFileName;

    // Open the file for writing
    std::ofstream ofs(filepath);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for saving: " + filepath);
    }

    const Table& table = it->second;

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

    ofs.close();
    std::cout << "Table '" << tableName << "' saved to '" << filepath << "' successfully." << std::endl;
}


void Database::loadFromFile(const std::string& command) {
    // Split the command on " AS " (case-sensitive match)
    std::string command_pr = removeTrailingSemicolon(trim(command));
    size_t asPos = command_pr.find(" AS ");
    std::string csvFileName, tableName;

    if (asPos != std::string::npos) {
        // "AS" is present: Extract CSV file name and table name
        csvFileName = trim(command_pr.substr(0, asPos));
        tableName = trim(command_pr.substr(asPos + 4)); // Skip " AS "
    } else {
        // "AS" not present: Treat the CSV file name as the table name
        csvFileName = trim(command_pr);
        tableName = csvFileName.substr(0, csvFileName.find_last_of('.')); // Remove ".csv"
    }

    if (csvFileName.empty() || tableName.empty()) {
        throw std::runtime_error("Syntax error in LOAD command. Table name or CSV file name is missing.");
    }

    fmt::print("Filename is {}", csvFileName);

    // Construct the file path with .csv extension
    const std::string filepath = "../data/" + csvFileName;

    // Open the file for reading
    std::ifstream ifs(filepath);
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    // Check if the table already exists in memory
    if (tables.find(tableName) != tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' already exists in memory. Drop it first before loading.");
    }

    Table table;
    table.name = tableName;

    std::string line;

    // Read column headers
    if (std::getline(ifs, line)) {
        std::vector<std::string> columnHeaders = split(line, ',');
        for (const auto& header : columnHeaders) {
            Column column = {trim(header), DataType::VARCHAR}; // Default type: VARCHAR
            table.columns.push_back(column);
        }
    }

    // Read rows
    while (std::getline(ifs, line)) {
        std::vector<std::string> rowValues = split(line, ',');
        if (rowValues.size() != table.columns.size()) {
            throw std::runtime_error("Row data does not match column count in table '" + tableName + "'.");
        }

        Row row;
        for (const auto& value : rowValues) {
            row.values.push_back(trim(value)); // Add trimmed values to the row
        }
        table.rows.push_back(row);
    }

    // Add the table to the database
    tables[tableName] = table;

    ifs.close();
    std::cout << "Table '" << tableName << "' loaded successfully from '" << filepath << "'." << std::endl;
}
