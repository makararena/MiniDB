#include <iostream>
#include "database.h"
#include <sstream>
#include <fstream>
#include <fmt/format.h>

#include "condition.h"
#include "file_io.h"
#include "utils.h"

// Database class implementation

// This is the constructor for the Database class.
// It is defined as empty, meaning it doesn’t perform any initialization or setup tasks when a Database object is created.
Database::Database() {}

// This function is a member of the Database class
DataType Database::parseDataType(const std::string& typeStr) {
    std::string upperTypeStr = toUpperCase(typeStr);
    if (upperTypeStr == "INTEGER") return DataType::INTEGER;
    if (upperTypeStr == "VARCHAR") return DataType::VARCHAR;
    if (upperTypeStr == "DATE") return DataType::DATE;
    if (upperTypeStr == "CHAR") return DataType::CHAR;
    if (upperTypeStr == "FLOAT") return DataType::FLOAT;
    throw std::runtime_error("Unsupported data type: " + typeStr);
}

// Interface for various command executions
void Database::executeCommand(const std::string& command) {
    std::string trimmedCommand = trim(command);
    std::stringstream ss(trimmedCommand);
    std::string operation;
    ss >> operation;

    std::string restOfCommand;
    std::getline(ss, restOfCommand);
    restOfCommand = trim(restOfCommand);

    std::string upperOperation = toUpperCase(operation);
    if (upperOperation == "CREATE") {
        createTable(restOfCommand);
    } else if (upperOperation == "DROP") {
        dropTable(restOfCommand);
    } else if (upperOperation == "INSERT") {
        insertInto(restOfCommand);
    } else if (upperOperation == "SELECT") {
        selectFrom(restOfCommand);
    } else if (upperOperation == "SAVE") {
        saveToFile(restOfCommand);
    }
    else if (upperOperation == "LOAD") {
        loadFromFile(restOfCommand);
    }

    else {
        throw std::runtime_error("Unknown command.");
    }
}

void Database::createTable(const std::string& command) {
    std::stringstream ss(command);
    std::string keyword;
    ss >> keyword;

    if (toUpperCase(keyword) != "TABLE") {
        throw std::runtime_error("Syntax error in CREATE TABLE command.");
    }

    std::string tableName;
    ss >> tableName;

    if (tables.find(tableName) != tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' already exists.");
    }

    std::string columnsDef;
    std::getline(ss, columnsDef);
    columnsDef = removeTrailingSemicolon(trim(columnsDef));

    if (columnsDef.front() != '(' || columnsDef.back() != ')') {
        throw std::runtime_error("Syntax error in CREATE TABLE command.");
    }

    columnsDef = columnsDef.substr(1, columnsDef.size() - 2);

    std::vector<std::string> columnDefs = split(columnsDef, ',');
    Table table;
    table.name = tableName;

    for (auto& colDef : columnDefs) {
        colDef = trim(colDef);

        if (toUpperCase(colDef).starts_with("PRIMARY KEY")) {
            size_t start = colDef.find('(') + 1;
            size_t end = colDef.find(')');
            std::string pkColumns = colDef.substr(start, end - start);
            std::vector<std::string> primaryKeys = split(pkColumns, ',');
            for (auto& pk : primaryKeys) {
                table.primaryKeys.push_back(trim(pk));
            }
        } else if (toUpperCase(colDef).starts_with("FOREIGN KEY")) {
            size_t start = colDef.find('(') + 1;
            size_t end = colDef.find(')');
            std::string column = trim(colDef.substr(start, end - start));

            size_t refStart = colDef.find("REFERENCES") + 10;
            std::string reference = trim(colDef.substr(refStart));
            table.foreignKeys[column] = reference;
        } else {
            std::stringstream colStream(colDef);
            std::string colName, colTypeStr;
            colStream >> colName >> colTypeStr;

            if (colName.empty() || colTypeStr.empty()) {
                throw std::runtime_error("Syntax error in column definition.");
            }

            DataType colType = parseDataType(colTypeStr);
            Column column = {colName, colType};
            table.columns.push_back(column);
        }
    }

    tables[tableName] = table;
    fmt::print("Table '{}' created successfully.\n", tableName);
}



void Database::dropTable(const std::string& command) {
    // Expected format: "TABLE table_name;"
    std::stringstream ss(removeTrailingSemicolon(trim(command)));
    std::string keyword;
    ss >> keyword;

    if (toUpperCase(keyword) != "TABLE") {
        throw std::runtime_error("Syntax error in DROP TABLE command.");
    }

    std::string tableName;
    ss >> tableName;

    auto it = tables.find(tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' does not exist.");
    }

    tables.erase(it);
    fmt::print("Table '{}' dropped successfully.\n", tableName);
}

void Database::insertInto(const std::string& command) {
    std::stringstream ss(command);
    std::string keyword;
    ss >> keyword;

    if (toUpperCase(keyword) != "INTO") {
        throw std::runtime_error("Syntax error in INSERT INTO command.");
    }

    std::string tableName;
    ss >> tableName;

    auto it = tables.find(tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' does not exist.");
    }

    Table& table = it->second;

    ss >> keyword;
    if (toUpperCase(keyword) != "VALUES") {
        throw std::runtime_error("Syntax error in INSERT INTO command.");
    }

    std::string valuesDef;
    std::getline(ss, valuesDef);
    valuesDef = removeTrailingSemicolon(trim(valuesDef));

    if (valuesDef.front() != '(' || valuesDef.back() != ')') {
        throw std::runtime_error("Syntax error in INSERT INTO command.");
    }

    valuesDef = valuesDef.substr(1, valuesDef.size() - 2); // Remove parentheses
    std::vector<std::string> values = split(valuesDef, ',');
    if (values.size() != table.columns.size()) {
        throw std::runtime_error("Column count doesn't match value count.");
    }

    // Create a new row
    Row row;
    for (size_t i = 0; i < values.size(); ++i) {
        std::string val = trim(values[i]);

        // Convert the string value to the correct type
        if (table.columns[i].type == DataType::INTEGER) {
            row.values.push_back(std::stoi(val));
        } else if (table.columns[i].type == DataType::FLOAT) {
            row.values.push_back(std::stof(val));
        } else if (table.columns[i].type == DataType::CHAR) {
            if (val.size() != 3 || val.front() != '\'' || val.back() != '\'') {
                throw std::runtime_error("Invalid CHAR value format.");
            }
            row.values.push_back(val[1]);
        } else if (table.columns[i].type == DataType::VARCHAR || table.columns[i].type == DataType::DATE) {
            if (val.front() == '\'' && val.back() == '\'') {
                row.values.push_back(val.substr(1, val.size() - 2)); // Remove quotes
            } else {
                throw std::runtime_error("Invalid VARCHAR or DATE value format.");
            }
        }
    }

    // Enforce primary key and foreign key constraints
    enforcePrimaryKey(table, row);
    enforceForeignKeys(table, row);

    // Add the validated row to the table
    table.rows.push_back(row);
    fmt::print("Row inserted into '{}' successfully.\n", tableName);
}

// ------------------------------------------------------------------------------------------------------------------
void Database::selectFrom(const std::string& command) {
    // Remove trailing semicolon and trim the command
    std::string cleanedCommand = removeTrailingSemicolon(trim(command));

    // Find the positions of "FROM" and "WHERE" in the query
    std::size_t wherePos = cleanedCommand.find(" WHERE ");
    std::size_t fromPos = cleanedCommand.find(" FROM ");

    // Ensure "FROM" is present; it's mandatory
    if (fromPos == std::string::npos) {
        throw std::runtime_error("Syntax error in SELECT command. Missing 'FROM'.");
    }

    // Extract the columns part (before "FROM") and table name (after "FROM")
    std::string columnsPart = trim(cleanedCommand.substr(0, fromPos));
    std::string tablePart = trim(
        cleanedCommand.substr(fromPos + 6,
                              (wherePos == std::string::npos ? std::string::npos : wherePos - (fromPos + 6)))
    );

    // Extract the WHERE clause (if present)
    std::string wherePart = (wherePos != std::string::npos) ? trim(cleanedCommand.substr(wherePos + 7)) : "";

    // Check if the specified table exists
    auto it = tables.find(tablePart);
    if (it == tables.end()) {
        throw std::runtime_error("Table '" + tablePart + "' does not exist.");
    }

    // Reference to the target table
    Table& table = it->second;

    // Determine which columns to select
    std::vector<int> colIndices;
    bool selectAll = (columnsPart == "*"); // Check for wildcard

    if (selectAll) {
        // Select all columns
        for (size_t i = 0; i < table.columns.size(); ++i) {
            colIndices.push_back(i);
        }
    } else {
        // Parse the list of specified columns
        std::vector<std::string> selectedColumns = split(columnsPart, ',');
        for (auto& colName : selectedColumns) {
            colName = trim(colName);
            bool found = false;

            // Find the index of each specified column
            for (size_t i = 0; i < table.columns.size(); ++i) {
                if (table.columns[i].name == colName) {
                    colIndices.push_back(i);
                    found = true;
                    break;
                }
            }

            if (!found) {
                throw std::runtime_error("Column '" + colName + "' does not exist in table '" + tablePart + "'.");
            }
        }
    }

    // Filter rows based on the WHERE clause (if provided)
    std::vector<Row> filteredRows = table.rows;
    if (!wherePart.empty()) {
        auto conditions = parseWhereClause(wherePart); // Parse the WHERE clause
        filteredRows = filterRows(table, conditions);  // Apply the conditions
    }

    // Handle case where no rows match the conditions
    if (filteredRows.empty()) {
        fmt::print("| No matching rows |\n");
        return;
    }

    // Prepare for column alignment by calculating maximum column widths
    std::vector<size_t> colWidths(colIndices.size(), 0);

    // Calculate column widths (headers and data)
    for (size_t i = 0; i < colIndices.size(); ++i) {
        colWidths[i] = table.columns[colIndices[i]].name.length(); // Start with header length

        for (const auto& row : filteredRows) {
            const auto& value = row.values[colIndices[i]];
            size_t valueLength = 0;

            // Determine value length based on type
            if (std::holds_alternative<int>(value)) {
                valueLength = std::to_string(std::get<int>(value)).length();
            } else if (std::holds_alternative<float>(value)) {
                valueLength = fmt::format("{:.2f}", std::get<float>(value)).length();
            } else if (std::holds_alternative<std::string>(value)) {
                valueLength = std::get<std::string>(value).length();
            }

            colWidths[i] = std::max(colWidths[i], valueLength); // Update max width
        }
    }

    // Print column headers
    fmt::print("|");
    for (size_t i = 0; i < colIndices.size(); ++i) {
        fmt::print(" {:<{}} |", table.columns[colIndices[i]].name, colWidths[i]);
    }
    fmt::print("\n");

    // Print separator line
    fmt::print("|");
    for (size_t i = 0; i < colIndices.size(); ++i) {
        fmt::print(" {:-<{}} |", "", colWidths[i]);
    }
    fmt::print("\n");

    // Print rows
    for (const auto& row : filteredRows) {
        fmt::print("|");
        for (size_t i = 0; i < colIndices.size(); ++i) {
            const auto& value = row.values[colIndices[i]];

            if (std::holds_alternative<int>(value)) {
                fmt::print(" {:<{}} |", std::get<int>(value), colWidths[i]);
            } else if (std::holds_alternative<float>(value)) {
                fmt::print(" {:<{}} |", fmt::format("{:.2f}", std::get<float>(value)), colWidths[i]);
            } else if (std::holds_alternative<std::string>(value)) {
                fmt::print(" {:<{}} |", std::get<std::string>(value), colWidths[i]);
            }
        }
        fmt::print("\n");
    }
}


// Helper to handle JOIN queries
void Database::selectWithJoin(const std::string& command) {
    std::size_t joinPos = command.find(" JOIN ");
    std::string leftPart = trim(command.substr(0, joinPos));
    std::string rightPart = trim(command.substr(joinPos + 6));

    std::size_t fromPos = leftPart.find(" FROM ");
    if (fromPos == std::string::npos) {
        throw std::runtime_error("Syntax error: Missing FROM clause in JOIN query.");
    }

    std::string columnsPart = trim(leftPart.substr(0, fromPos)); // Columns to select
    std::string leftTable = trim(leftPart.substr(fromPos + 6));

    std::size_t onPos = rightPart.find(" ON ");
    if (onPos == std::string::npos) {
        throw std::runtime_error("JOIN must have an ON condition.");
    }

    std::string rightTable = trim(rightPart.substr(0, onPos));
    std::string onCondition = trim(rightPart.substr(onPos + 4));

    std::size_t eqPos = onCondition.find("=");
    if (eqPos == std::string::npos) {
        throw std::runtime_error("ON condition must use '='.");
    }
    std::string leftColumn = trim(onCondition.substr(0, eqPos));
    std::string rightColumn = trim(onCondition.substr(eqPos + 1));

    auto leftIt = tables.find(leftTable);
    auto rightIt = tables.find(rightTable);

    if (leftIt == tables.end() || rightIt == tables.end()) {
        throw std::runtime_error("One of the tables in the JOIN does not exist.");
    }

    const Table& left = leftIt->second;
    const Table& right = rightIt->second;

    auto findColumnIndex = [](const Table& table, const std::string& columnName) -> size_t {
        for (size_t i = 0; i < table.columns.size(); ++i) {
            if (table.columns[i].name == columnName) {
                return i;
            }
        }
        throw std::runtime_error("Column '" + columnName + "' does not exist in table '" + table.name + "'.");
    };

    size_t leftColIndex = findColumnIndex(left, leftColumn);
    size_t rightColIndex = findColumnIndex(right, rightColumn);

    // Perform JOIN
    for (const auto& leftRow : left.rows) {
        for (const auto& rightRow : right.rows) {
            if (leftRow.values[leftColIndex] == rightRow.values[rightColIndex]) {
                fmt::print("| {} | {} |\n", leftRow.values[leftColIndex], rightRow.values[rightColIndex]);
            }
        }
    }
}


void Database::enforcePrimaryKey(const Table& table, const Row& row) {
    for (const auto& pk : table.primaryKeys) {
        auto pkColIt = std::find_if(
            table.columns.begin(),
            table.columns.end(),
            [&](const Column& col) { return col.name == pk; }
        );

        if (pkColIt == table.columns.end()) {
            throw std::runtime_error("Primary key column '" + pk + "' does not exist.");
        }

        size_t pkIndex = std::distance(table.columns.begin(), pkColIt);
        for (const auto& existingRow : table.rows) {
            if (existingRow.values[pkIndex] == row.values[pkIndex]) {
                throw std::runtime_error("Duplicate value for primary key '" + pk + "'.");
            }
        }
    }
}


// Enforce foreign key constraints
void Database::enforceForeignKeys(const Table& table, const Row& row) {
    for (const auto& [fkColumn, reference] : table.foreignKeys) {
        auto fkColIt = std::find_if(
            table.columns.begin(),
            table.columns.end(),
            [&](const Column& col) { return col.name == fkColumn; }
        );

        if (fkColIt == table.columns.end()) {
            throw std::runtime_error("Foreign key column '" + fkColumn + "' does not exist.");
        }

        size_t fkIndex = std::distance(table.columns.begin(), fkColIt);

        auto refTableCol = split(reference, '.');
        const std::string& refTableName = refTableCol[0];
        const std::string& refColumnName = refTableCol[1];

        auto refTableIt = tables.find(refTableName);
        if (refTableIt == tables.end()) {
            throw std::runtime_error("Referenced table '" + refTableName + "' does not exist.");
        }

        const Table& refTable = refTableIt->second;

        auto refColIt = std::find_if(
            refTable.columns.begin(),
            refTable.columns.end(),
            [&](const Column& col) { return col.name == refColumnName; }
        );

        if (refColIt == refTable.columns.end()) {
            throw std::runtime_error("Referenced column '" + refColumnName + "' does not exist.");
        }

        size_t refColIndex = std::distance(refTable.columns.begin(), refColIt);

        bool found = false;
        for (const auto& refRow : refTable.rows) {
            if (refRow.values[refColIndex] == row.values[fkIndex]) {
                found = true;
                break;
            }
        }

        if (!found) {
            throw std::runtime_error("Foreign key constraint violated for column '" + fkColumn + "'.");
        }
    }
}