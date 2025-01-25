#include <iostream>
#include <sstream>
#include <fstream>

#include "database.h"
#include "condition.h"
#include "utils.h"
#include "fmt/color.h"

Database::Database() {
    // Constructor: no special initialization required
}

DataType Database::parseDataType(const std::string& typeStr) {
    std::string upperTypeStr = toCase(typeStr, CaseType::UPPER);
    if (upperTypeStr == "INTEGER") return DataType::INTEGER;
    if (upperTypeStr == "VARCHAR") return DataType::VARCHAR;
    if (upperTypeStr == "DATE")    return DataType::DATE;
    if (upperTypeStr == "CHAR")    return DataType::CHAR;
    if (upperTypeStr == "FLOAT")   return DataType::FLOAT;
    throw std::runtime_error("Unsupported data type: " + typeStr);
}

void Database::executeCommand(const std::string& command) {
    std::string trimmedCommand = removeTrailingSemicolon(trim(command));

    // Normalize only the operation (keyword)
    std::string normalizedCommand = normalizeKeywords(trimmedCommand);


    std::stringstream ss(normalizedCommand); // What is stringstream https://stackoverflow.com/questions/20594520/what-exactly-does-stringstream-do
    std::string normalizedOperation;
    ss >> normalizedOperation; // The first keyword (SELECT, CREATE, etc.)

    // Get the rest of the command
    std::string restOfCommand;
    std::getline(ss, restOfCommand);
    restOfCommand = trim(restOfCommand); // Preserve original case for the rest

    if (normalizedOperation == "SELECT") {
        selectFrom(restOfCommand);
    } else if (normalizedOperation == "CREATE") {
        createTable(restOfCommand);
    } else if (normalizedOperation == "DROP") {
        dropTable(restOfCommand);
    } else if (normalizedOperation == "INSERT") {
        insertInto(restOfCommand);
    } else if (normalizedOperation == "SAVE") {
        saveToFile(restOfCommand);
    } else if (normalizedOperation == "LOAD") {
        loadFromFile(restOfCommand);
    } else if (normalizedOperation == "LIST" && restOfCommand == "TABLES") {
        listTables();
    }
    // https://cplusplus.com/reference/string/string/rfind/
    else if (normalizedOperation == "DELETE" && restOfCommand.rfind("FILE", 0) == 0) {
        deleteFile(restOfCommand.substr(5)); // Pass the file name part
    }
    else {
        throw std::runtime_error("Unknown command: " + normalizedOperation);
    }
}

// ---------------------------------------------------------------------------------------
void Database::createTable(const std::string& command) {
    // Expected format: CREATE TABLE table_name (colName colType, colName colType, ...);
    std::stringstream ss(command);
    std::string keyword;
    ss >> keyword;  // Should be "TABLE"

    if (toCase(keyword, CaseType::UPPER)  != "TABLE") {
        throw std::runtime_error("Syntax error in CREATE TABLE command.");
    }

    // Read the table name
    std::string tableName;
    ss >> tableName;

    // Check if table already exists
    if (tables.find(tableName) != tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' already exists.");
    }

    // Read the column definitions between parentheses (...)
    std::string columnsDef;
    std::getline(ss, columnsDef);
    columnsDef = removeTrailingSemicolon(trim(columnsDef));

    // Basic syntax check for parentheses
    if (columnsDef.size() < 2 || columnsDef.front() != '(' || columnsDef.back() != ')') {
        throw std::runtime_error("Syntax error in CREATE TABLE command.");
    }

    // Remove outer parentheses
    columnsDef = columnsDef.substr(1, columnsDef.size() - 2);

    // Split by commas to get each "name type" pair
    std::vector<std::string> columnDefs = split(columnsDef, ',');

    // Construct the table
    Table table;
    table.name = tableName;

    // Parse each column definition
    for (auto& colDef : columnDefs) {
        colDef = trim(colDef);
        std::stringstream colStream(colDef);

        std::string colName;
        std::string colTypeStr;
        colStream >> colName >> colTypeStr;

        if (colName.empty() || colTypeStr.empty()) {
            throw std::runtime_error("Syntax error in column definition: '" + colDef + "'.");
        }

        // Convert string type to enum
        DataType colType = parseDataType(colTypeStr);

        // Add column to table
        Column column = { colName, colType };
        table.columns.push_back(column);
    }

    // Store the new table in the database
    tables[tableName] = table;
    fmt::print("Table '{}' created successfully.\n", tableName);
}

void Database::dropTable(const std::string& command) {
    // Expected format: DROP TABLE table_name;
    std::stringstream ss(removeTrailingSemicolon(trim(command)));
    std::string keyword;
    ss >> keyword;  // Should be "TABLE"

    if (toCase(keyword, CaseType::UPPER) != "TABLE") {
        throw std::runtime_error("Syntax error in DROP TABLE command.");
    }

    // Read the table name
    std::string tableName;
    ss >> tableName;

    // Check existence
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' does not exist.");
    }

    // Erase from the map
    tables.erase(it);
    fmt::print("Table '{}' dropped successfully.\n", tableName);
}

void Database::insertInto(const std::string& command) {
    // Expected format: INSERT INTO table_name VALUES (...);
    std::stringstream ss(command);
    std::string keyword;
    ss >> keyword; // Should be "INTO"

    if (toCase(keyword, CaseType::UPPER) != "INTO") {
        throw std::runtime_error("Syntax error in INSERT INTO command.");
    }

    // Read the table name
    std::string tableName;
    ss >> tableName;

    // Check existence
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' does not exist.");
    }
    Table& table = it->second;

    // Read the next keyword, should be "VALUES"
    ss >> keyword;
    if (toCase(keyword, CaseType::UPPER) != "VALUES") {
        throw std::runtime_error("Syntax error in INSERT INTO command. Missing 'VALUES'.");
    }

    // Parse values in parentheses
    std::string valuesDef;
    std::getline(ss, valuesDef);
    valuesDef = removeTrailingSemicolon(trim(valuesDef));

    if (valuesDef.size() < 2 || valuesDef.front() != '(' || valuesDef.back() != ')') {
        throw std::runtime_error("Syntax error in INSERT INTO command.");
    }

    // Strip the outer parentheses
    valuesDef = valuesDef.substr(1, valuesDef.size() - 2);

    // Split by commas
    std::vector<std::string> values = split(valuesDef, ',');
    if (values.size() != table.columns.size()) {
        throw std::runtime_error("Column count doesn't match value count.");
    }

    // Create a new row
    Row row;
    for (size_t i = 0; i < values.size(); ++i) {
        std::string val = trim(values[i]);

        // Convert string to the column’s data type
        switch (table.columns[i].type) {
            case DataType::INTEGER: {
                row.values.push_back(std::stoi(val)); // stoi - string to integer
                break;
            }
            case DataType::FLOAT: {
                row.values.push_back(std::stof(val)); // stof - string to float
                break;
            }
            case DataType::CHAR: {
                // Expecting a single quoted character, e.g. 'a'
                if (val.size() != 3 || val.front() != '\'' || val.back() != '\'') {
                    throw std::runtime_error("Invalid CHAR format (expected single quoted character).");
                }
                row.values.push_back(val[1]);
                break;
            }
            case DataType::VARCHAR:
            case DataType::DATE: {
                // Expecting a single quoted string, e.g. 'Hello'
                if (val.front() == '\'' && val.back() == '\'') {
                    row.values.push_back(val.substr(1, val.size() - 2));
                } else {
                    throw std::runtime_error("Invalid string or date format (must be in quotes).");
                }
                break;
            }
            default:
                throw std::runtime_error("Unknown data type encountered.");
        }
    }

    // Add the row to the table
    table.rows.push_back(row);
    fmt::print("Row inserted into '{}' successfully.\n", tableName);
}

// ---------------------------------------------------------------------------------------
void Database::selectFrom(const std::string& command) {
    // Expected format (basic version):
    //   SELECT col1, col2 FROM tableName
    //   [WHERE conditions]
    //   [ORDER BY colName [ASC|DESC]]
    //   [LIMIT number];

    std::string cleanedCommand = removeTrailingSemicolon(trim(command));

    // Let's find positions of FROM, WHERE, ORDER BY, and LIMIT
    std::size_t fromPos   = cleanedCommand.find(" FROM ");
    std::size_t wherePos  = cleanedCommand.find(" WHERE ");
    std::size_t orderPos  = cleanedCommand.find(" ORDER BY ");
    std::size_t limitPos  = cleanedCommand.find(" LIMIT ");

    if (fromPos == std::string::npos) {
        throw std::runtime_error("Syntax error in SELECT command (missing 'FROM').");
    }

    // 1) Extract the SELECT columns part
    //    substring from [0..fromPos)
    std::string columnsPart = trim(cleanedCommand.substr(0, fromPos));

    // 2) Identify the portion that starts after FROM
    //    If there's a WHERE, ORDER BY, or LIMIT, we only take up to the first one we find
    //    We'll do this by identifying the earliest of wherePos, orderPos, limitPos after fromPos
    std::size_t tablePartEnd = cleanedCommand.size(); // default end

    // find earliest of wherePos, orderPos, limitPos that is > fromPos
    std::vector<std::size_t> markers;
    if (wherePos  != std::string::npos && wherePos  > fromPos) markers.push_back(wherePos);
    if (orderPos  != std::string::npos && orderPos  > fromPos) markers.push_back(orderPos);
    if (limitPos  != std::string::npos && limitPos  > fromPos) markers.push_back(limitPos);

    // if we found any markers, pick the smallest
    if (!markers.empty()) {
        // so the std::min_element returns iterator and if we want to actually get the real values we can just use *
        tablePartEnd = *std::min_element(markers.begin(), markers.end());
    }

    // The table name portion: fromPos + 6 (skip " FROM ") up to tablePartEnd
    std::string tablePart = trim(
        cleanedCommand.substr(fromPos + 6, tablePartEnd - (fromPos + 6))
    );

    // 3) Extract the WHERE part (if any)
    std::string wherePart;
    if (wherePos != std::string::npos) {
        // from wherePos+7 (" WHERE ") until either ORDER BY or LIMIT or end
        std::size_t whereEnd = cleanedCommand.size();
        // find if there's an ORDER BY or LIMIT after the WHERE
        std::vector<std::size_t> afterWhereMarkers;
        if (orderPos != std::string::npos && orderPos > wherePos) afterWhereMarkers.push_back(orderPos);
        if (limitPos != std::string::npos && limitPos > wherePos) afterWhereMarkers.push_back(limitPos);
        if (!afterWhereMarkers.empty()) {
            whereEnd = *std::min_element(afterWhereMarkers.begin(), afterWhereMarkers.end());
        }
        wherePart = trim(cleanedCommand.substr(wherePos + 7, whereEnd - (wherePos + 7)));
    }

    // 4) Extract the ORDER BY part (if any)
    // 1) Extract substring after "ORDER BY " until "LIMIT" or the end
    std::vector<std::pair<std::string, bool>> orderByColumns;
    std::string orderByPart;
    if (orderPos != std::string::npos) {
        std::size_t orderEnd = (limitPos == std::string::npos)
                               ? cleanedCommand.size()
                               : limitPos;
        orderByPart = trim(cleanedCommand.substr(orderPos + 9, orderEnd - (orderPos + 9)));

        // 2) Split the part by commas first -> multiple columns
        std::vector<std::string> columns = split(orderByPart, ',');
        for (auto& col : columns) {
            // e.g. "age" or "name DESC"
            col = trim(col);

            // 3) Split each column by spaces to find optional ASC/DESC
            std::vector<std::string> orderTokens = split(col, ' ');
            if (!orderTokens.empty()) {
                std::string columnName = trim(orderTokens[0]);
                bool isDesc = false;

                // If second token exists, check if it's DESC
                if (orderTokens.size() > 1) {
                    std::string direction = toCase(trim(orderTokens[1]), CaseType::UPPER);
                    if (direction == "DESC") {
                        isDesc = true;
                    }
                }

                // 4) Push result into `orderByColumns`
                orderByColumns.emplace_back(columnName, isDesc);
            }
        }
    }

    // 5) Extract the LIMIT part (if any)
    int limitValue = -1; // -1 means no limit
    if (limitPos != std::string::npos) {
        // substring from limitPos+6 (" LIMIT ") to end
        std::string limitStr = trim(cleanedCommand.substr(limitPos + 7));
        if (!limitStr.empty()) {
            limitValue = std::stoi(limitStr);
            if (limitValue < 0) {
                throw std::runtime_error("LIMIT value cannot be negative.");
            }
        }
    }

    // 6) Check table existence
    auto it = tables.find(tablePart);
    if (it == tables.end()) {
        throw std::runtime_error("Table '" + tablePart + "' does not exist.");
    }
    Table& table = it->second; // reference to the table

    // 7) Determine which columns to select
    std::vector<int> colIndices;
    bool selectAll = (columnsPart == "*");

    if (selectAll) {
        // Wildcard: select all columns
        for (size_t i = 0; i < table.columns.size(); ++i) {
            // https://www.geeksforgeeks.org/static_cast-in-cpp/
            colIndices.push_back(static_cast<int>(i));
        }
    } else {
        // Split column list by comma
        std::vector<std::string> selectedCols = split(columnsPart, ',');
        for (auto& col : selectedCols) {
            col = trim(col);
            bool found = false;
            for (size_t i = 0; i < table.columns.size(); ++i) {
                if (table.columns[i].name == col) {
                    // https://www.geeksforgeeks.org/static_cast-in-cpp/
                    colIndices.push_back(static_cast<int>(i));
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Column '" + col + "' not found in table '" + tablePart + "'.");
            }
        }
    }

    // 8) Apply WHERE (filter rows)
    std::vector<Row> filteredRows = table.rows;
    if (!wherePart.empty()) {
        auto conditions = parseWhereClause(wherePart);
        filteredRows = filterRows(table, conditions);
    }

    if (!orderByColumns.empty()) {
        std::sort(filteredRows.begin(), filteredRows.end(),
            [&table, &orderByColumns](const Row& a, const Row& b) {
                // Compare row A and row B column by column
                for (const auto& [colName, isDesc] : orderByColumns) {
                    // Take the column iterator with the same name as in orderByColumns
                    auto colIt = std::find_if(
                        table.columns.begin(),
                        table.columns.end(),
                        [&](const Column& c){ return c.name == colName; }
                    );

                    if (colIt == table.columns.end()) {
                        throw std::runtime_error("Column '" + colName + "' not found in table.");
                    }

                    // Convert iterator to an index (colIndex).
                    size_t colIndex = std::distance(table.columns.begin(), colIt);
                    const Value& valA = a.values[colIndex];
                    const Value& valB = b.values[colIndex];

                    // If valA != valB, decide ordering. If they are equal, check next column.
                    if (valA != valB) {
                        // int
                        if (std::holds_alternative<int>(valA)) {
                            int va = std::get<int>(valA);
                            int vb = std::get<int>(valB);
                            return isDesc ? (va > vb) : (va < vb);
                        }
                        // std::string
                        else if (std::holds_alternative<std::string>(valA)) {
                            const std::string& sa = std::get<std::string>(valA);
                            const std::string& sb = std::get<std::string>(valB);
                            return isDesc ? (sa > sb) : (sa < sb);
                        }
                        // float
                        else if (std::holds_alternative<float>(valA)) {
                            float fa = std::get<float>(valA);
                            float fb = std::get<float>(valB);
                            return isDesc ? (fa > fb) : (fa < fb);
                        }
                        // char
                        else if (std::holds_alternative<char>(valA)) {
                            char ca = std::get<char>(valA);
                            char cb = std::get<char>(valB);
                            return isDesc ? (ca > cb) : (ca < cb);
                        }
                        else {
                            throw std::runtime_error("Unhandled data type in sorting logic.");
                        }
                    }
                }

                // If all compared columns are equal, retain original orderю
                return false;
            }
        );
    }

    // 10) Apply LIMIT if specified
    if (limitValue >= 0 && static_cast<size_t>(limitValue) < filteredRows.size()) {
        filteredRows.resize(limitValue); // https://www.geeksforgeeks.org/vector-resize-c-stl/
    }

    // 12) Format and print the results

    // Create a vector of size of number of columns with 0 to save all
    // max width for each column for the proper print
    std::vector<size_t> colWidths(colIndices.size(), 0);

    // Compute max width for each column
    for (size_t i = 0; i < colIndices.size(); ++i) {
        colWidths[i] = table.columns[colIndices[i]].name.size();
        for (const auto& row : filteredRows) {
            const auto& value = row.values[colIndices[i]];
            size_t valueLength = 0;
            if (std::holds_alternative<int>(value)) {
                valueLength = std::to_string(std::get<int>(value)).size();
            } else if (std::holds_alternative<float>(value)) {
                valueLength = fmt::format("{:.2f}", std::get<float>(value)).size();
            } else if (std::holds_alternative<std::string>(value)) {
                valueLength = std::get<std::string>(value).size();
            } else {
                // char
                valueLength = 1;
            }
            if (valueLength > colWidths[i]) {
                colWidths[i] = valueLength;
            }
        }
    }

    // Print header
    fmt::print("|");
    for (size_t i = 0; i < colIndices.size(); ++i) {
        // OpenAI model code (haven't found this thing in documentation, but it works :) )
        fmt::print(" {:<{}} |", table.columns[colIndices[i]].name, colWidths[i]);
    }
    fmt::print("\n");

    // Print separator
    fmt::print("|");
    for (size_t i = 0; i < colIndices.size(); ++i) {
        // OpenAI model code (haven't found this thing in documentation, but it works :) )
        fmt::print(" {:-<{}} |", "", colWidths[i]);
    }
    fmt::print("\n");

    // Print rows
    for (const auto& row : filteredRows) {
        fmt::print("|");
        for (size_t i = 0; i < colIndices.size(); ++i) {
            const auto& value = row.values[colIndices[i]];
            if (std::holds_alternative<int>(value)) {
                // OpenAI model code (haven't found this thing in documentation, but it works :) )
                fmt::print(" {:<{}} |", std::get<int>(value), colWidths[i]);
            } else if (std::holds_alternative<float>(value)) {
                // OpenAI model code (haven't found this thing in documentation, but it works :) )
                fmt::print(" {:<{}} |", fmt::format("{:.2f}", std::get<float>(value)), colWidths[i]);
            } else if (std::holds_alternative<std::string>(value)) {
                // OpenAI model code (haven't found this thing in documentation, but it works :) )
                fmt::print(" {:<{}} |", std::get<std::string>(value), colWidths[i]);
            } else {
                // char
                // OpenAI model code (haven't found this thing in documentation, but it works :) )
                fmt::print(" {:<{}} |", std::get<char>(value), colWidths[i]);
            }
        }
        fmt::print("\n");
    }
}

void Database::listTables() {
    if (tables.empty()) {
        std::cout << "No tables currently loaded in memory.\n";
        return;
    }

    std::cout << "Tables currently in memory:\n";
    for (const auto& pair : tables) {
        const std::string& tableName = pair.first;
        const Table& table = pair.second;

        std::cout << "- Table Name: " << tableName << "\n";
        std::cout << "  Columns: ";
        for (size_t i = 0; i < table.columns.size(); ++i) {
            std::cout << table.columns[i].name;
            if (i < table.columns.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "\n  Number of Rows: " << table.rows.size() << "\n";
    }
}

void Database::deleteFile(const std::string& rawFileName) {
    // Preprocess the file name: trim spaces and remove trailing semicolon
    std::string cleanedFileName = removeTrailingSemicolon(trim(rawFileName));

    if (cleanedFileName.empty()) {
        throw std::runtime_error("Syntax error in DELETE FILE command. File name is missing.");
    }

    // Construct the full file path
    const std::string fullFilePath = "./data" + cleanedFileName;

    // Attempt to delete the file
    if (std::remove(fullFilePath.c_str()) != 0) {
        throw std::runtime_error("Failed to delete file: " + fullFilePath + ". File may not exist.");
    }

    std::cout << "File '" << fullFilePath << "' deleted successfully." << std::endl;
}


