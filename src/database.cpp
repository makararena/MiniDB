// database.cpp

#include <iostream>
#include "database.h"
#include <sstream>
#include <fstream>
#include <fmt/format.h>

// Helper functions (https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring)
auto trim(const std::string& str) -> std::string{
    // std::size_t is an unsigned integer type specifically designed for representing sizes and indexes in arrays, strings, and other containers.
    // and also std::size_t can resperesnt much larger size than int

    std::size_t start = str.find_first_not_of(" \t\n\r"); // find first non-whitespace character ( ' ' (space); '\t' (tab); '\n' (newline); '\r' (carriage return))
    // If the string contains only whitespace, find_first_not_of will return std::string::npos.
    if (start == std::string::npos) return ""; // https://www.naukri.com/code360/library/string-npos-in-cpp (link to the what is npos) (represents the maximum constant for the string in c++)

    std::size_t end = str.find_last_not_of(" \t\n\r"); // scans the string from right to left looking for the last character than is not in the set of " \t\n\r"
    return str.substr(start, end - start + 1); // return the trimmed string
}

// Simple conversion to the upper string
std::string toUpperCase(const std::string& str) {
    std::string upperStr = str;
    std::transform(
        upperStr.begin(),
        upperStr.end(),
        upperStr.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return upperStr;
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens; //  A vector to store the resulting substrings
    std::string token; // A temporary string used to hold each substring as it is extracted
    std::stringstream ss(s); //  allows the string to be treated as a stream, enabling easy parsing

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

// Removes the trailing semicolon from a string, if present
std::string removeTrailingSemicolon(const std::string& str) {
    if (!str.empty() && str.back() == ';') {
        return str.substr(0, str.size() - 1); // Return the string without the last character
    }
    return str; // Return the original string if no semicolon is found
}

std::vector<std::pair<std::string, Condition>> parseWhereClause(const std::string& wherePart) {
    std::vector<std::pair<std::string, Condition>> conditions; // Resulting list of parsed conditions
    std::vector<std::string> tokens = split(wherePart, ' ');   // Split the `WHERE` clause into tokens by spaces

    std::string logicalOp; // Current logical operator between conditions (e.g., AND, OR, NOT)
    std::string currentColumn, currentOp, currentValue; // Components of a condition
    bool inCondition = false; // Indicates if we are building a condition
    bool inValue = false;     // Indicates if we are processing a multi-word value
    std::string valueBuffer;  // Temporary storage for multi-word string values

    // Iterate through each token in the WHERE clause
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::string token = tokens[i];

        // Check if the token is a logical operator
        if (token == "AND" || token == "OR" || token == "NOT") {
            if (inCondition) {
                // Finalize the current condition before moving to the next
                Condition cond{currentColumn, currentOp, currentValue, (logicalOp == "NOT")};
                conditions.emplace_back(logicalOp, cond); // Add the condition and logical operator to the list
                inCondition = false; // Reset for the next condition
            }
            logicalOp = token; // Update the current logical operator
        } else {
            // If it's not a logical operator, build a condition
            if (!inCondition) {
                // Start a new condition
                inCondition = true;
                currentColumn = token; // First token is the column name
            } else if (currentOp.empty()) {
                // Second token is the operator (e.g., =, >, <)
                currentOp = token;
            } else {
                // Remaining tokens are part of the value
                if (token.front() == '\'' || inValue) {
                    // If the value starts with a single quote or we are in a multi-word value
                    inValue = true; // Mark as inside a multi-word value
                    valueBuffer += (valueBuffer.empty() ? "" : " ") + token; // Append token to the value buffer
                    if (token.back() == '\'') {
                        // If the value ends with a single quote, finalize the value
                        currentValue = valueBuffer.substr(1, valueBuffer.size() - 2); // Strip the surrounding quotes
                        valueBuffer.clear(); // Clear the buffer
                        inValue = false;     // Exit multi-word value mode
                    }
                } else {
                    // If the value is a simple token (not quoted)
                    currentValue = token;
                }
            }
        }
    }

    // Finalize the last condition if any
    if (inCondition) {
        Condition cond{currentColumn, currentOp, currentValue, (logicalOp == "NOT")};
        conditions.emplace_back(logicalOp, cond); // Add the final condition
    }

    return conditions; // Return the list of parsed conditions
}

// Helper: Evaluate a condition for a single row
bool evaluateCondition(const Row& row, const Table& table, const Condition& cond) {
    auto colIt = std::find_if(
        table.columns.begin(),
        table.columns.end(),
        [&](const Column& col) { return col.name == cond.column; }
    );

    if (colIt == table.columns.end()) {
        throw std::runtime_error("Column '" + cond.column + "' does not exist.");
    }

    size_t colIndex = std::distance(table.columns.begin(), colIt);
    const Value& value = row.values[colIndex];

    bool result = false;
    try {
        if (std::holds_alternative<int>(value)) {
            int intValue = std::get<int>(value);
            int condValue = std::stoi(cond.value);
            if (cond.op == "=") result = (intValue == condValue);
            else if (cond.op == ">") result = (intValue > condValue);
            else if (cond.op == "<") result = (intValue < condValue);
        } else if (std::holds_alternative<float>(value)) {
            float floatValue = std::get<float>(value);
            float condValue = std::stof(cond.value);
            if (cond.op == "=") result = (floatValue == condValue);
            else if (cond.op == ">") result = (floatValue > condValue);
            else if (cond.op == "<") result = (floatValue < condValue);
        } else if (std::holds_alternative<char>(value)) {
            char charValue = std::get<char>(value);
            if (cond.op == "=") result = (charValue == cond.value[0]);
        } else if (std::holds_alternative<std::string>(value)) {
            const std::string& strValue = std::get<std::string>(value);
            if (cond.op == "=") result = (strValue == cond.value);
        }
    } catch (...) {
        throw std::runtime_error("Type mismatch in WHERE clause: Cannot compare '" + cond.value + "' to column '" + cond.column + "'");
    }

    return cond.negate ? !result : result;
}

// Helper: Apply WHERE clause to rows
std::vector<Row> filterRows(const Table& table, const std::vector<std::pair<std::string, Condition>>& conditions) {
    std::vector<Row> filteredRows;

    for (const auto& row : table.rows) {
        bool overallResult = (conditions.empty() || conditions[0].first.empty()) ? true : false;

        for (const auto& [logicalOp, cond] : conditions) {
            bool condResult = evaluateCondition(row, table, cond);

            if (logicalOp == "AND") {
                overallResult = overallResult && condResult;
            } else if (logicalOp == "OR") {
                overallResult = overallResult || condResult;
            } else {
                overallResult = condResult; // First condition
            }
        }

        if (overallResult) {
            filteredRows.push_back(row);
        }
    }

    return filteredRows;
}

// Database class implementation

// This is the constructor for the Database class.
// It is defined as empty, meaning it doesn’t perform any initialization or setup tasks when a Database object is created.
Database::Database() {}

// This function is a member of the Database class
DataType Database::parseDataType(const std::string& typeStr) {
    std::string upperTypeStr = toUpperCase(typeStr);
    if (upperTypeStr == "INTEGER") {
        return DataType::INTEGER;
    } else if (upperTypeStr == "VARCHAR") {
        return DataType::VARCHAR;
    } else if (upperTypeStr == "DATE") { // New case
        return DataType::DATE;
    } else if (upperTypeStr == "CHAR") { // New case
        return DataType::CHAR;
    } else if (upperTypeStr == "FLOAT") { // New case
        return DataType::FLOAT;
    } else {
        throw std::runtime_error("Unsupported data type: " + typeStr);
    }
}

// Interface for various command executions
void Database::executeCommand(const std::string& command) {
    // Removes any leading or trailing whitespace characters (spaces, tabs, newlines, etc.) from the input command string.
    std::string trimmedCommand = trim(command);

    //  Extracts the first word (or “token”) from the command string.
    // ss is an instance of the std::stringstream class, which is part of the C++ Standard Library. It is used to treat a string (trimmedCommand) as a stream, enabling operations like tokenization or formatted data extraction.
    std::stringstream ss(trimmedCommand);   //  Initialize a stringstream with the input string
    std::string operation;                  // Declare a variable to store the extracted token
    ss >> operation;                        // Extract the first token from the stringstream

    // fmt::print("Executing command {}\n", operation);

    std::string restOfCommand;
    std::getline(ss, restOfCommand); // Extract everything after the first word
    restOfCommand = trim(restOfCommand);

    // fmt::print("Executing the rest of the command {}\n", restOfCommand);

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
    } else if (upperOperation == "LOAD") {
        loadFromFile(restOfCommand);
    } else {
        throw std::runtime_error("Unknown command.");
    }
}

void Database::createTable(const std::string& command) {
    // Expected format: "TABLE table_name (column1 TYPE1, column2 TYPE2);"
    std::stringstream ss(command);
    std::string keyword;
    ss >> keyword;

    if (toUpperCase(keyword) != "TABLE") {
        std::cout << keyword;
        throw std::runtime_error("Syntax error in CREATE TABLE command.");
    }

    std::string tableName;
    ss >> tableName;

    // Check if table already exists
    if (tables.find(tableName) != tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' already exists.");
    }

    std::string columnsDef;
    std::getline(ss, columnsDef);
    columnsDef = removeTrailingSemicolon(trim(columnsDef));

    if (columnsDef.front() != '(' || columnsDef.back() != ')') {
        throw std::runtime_error("Syntax error in CREATE TABLE command.");
    }

    columnsDef = columnsDef.substr(1, columnsDef.size() - 2); // Remove parentheses

    // Parse columns
    std::vector<std::string> columnDefs = split(columnsDef, ','); // Split the column definitions into individual column strings using ',' as the delimiter.

    // Create a new table object
    Table table;
    table.name = tableName; // Set the name of the table to the provided tableName.

    // Loop through each column definition in columnDefs
    for (auto& colDef : columnDefs) {
        colDef = trim(colDef); // Remove leading and trailing whitespace from the column definition.

        std::stringstream colStream(colDef); // Create a stringstream to parse the column definition.
        std::string colName, colTypeStr;    // Variables to store the column name and its data type.

        colStream >> colName >> colTypeStr; // Extract the column name and data type from the column definition string.

        // Check if either the column name or type is missing.
        if (colName.empty() || colTypeStr.empty()) {
            throw std::runtime_error("Syntax error in column definition."); // Throw an error if the syntax is invalid.
        }

        // Convert the data type string into the corresponding enum value.
        DataType colType = parseDataType(colTypeStr);

        // Create a Column object with the parsed name and data type.
        Column column = {colName, colType};

        // Add the column to the table's list of columns.
        table.columns.push_back(column);
    }

    // Add the newly created table to the database's table map.
    tables[tableName] = table;

    // Print a success message indicating that the table was created successfully.
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
    // Expected format: "INTO table_name VALUES (value1, value2, ...);"
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

    // Remove parentheses from the values definition string
    // Example: If valuesDef = "(1, 'Alice')", this will strip the parentheses, leaving "1, 'Alice'"
    valuesDef = valuesDef.substr(1, valuesDef.size() - 2);

    // Split the remaining string by commas to get individual values
    // Example: "1, 'Alice'" → {"1", "'Alice'"}
    std::vector<std::string> values = split(valuesDef, ',');

    // Create a new Row object to store the parsed values
    Row row;

    // Loop through each value in the split list
    for (auto& valStr : values) {
        valStr = trim(valStr); // Remove any leading or trailing whitespace from the value string

        // Remove quotes from strings (if applicable)
        // Example: If valStr = "'Alice'", this will strip the single quotes, leaving "Alice"
        if (!valStr.empty() && valStr.front() == '\'' && valStr.back() == '\'') {
            valStr = valStr.substr(1, valStr.size() - 2);
        }

        // Add the processed value to the row's list of values
        row.values.push_back(valStr);
    }

    // Check that the number of values matches the number of columns in the table
    // Example: If the table has 2 columns but only 1 value is provided, this will throw an error
    if (row.values.size() != it->second.columns.size()) {
        throw std::runtime_error("Column count doesn't match value count.");
    }

    for (size_t i = 0; i < row.values.size(); ++i) {
        DataType colType = it->second.columns[i].type;  // Get the expected type
        std::string& valStr = std::get<std::string>(row.values[i]);  // Extract the string

        try {
            if (colType == DataType::INTEGER) {
                // Convert string to int and store it back in the variant
                row.values[i] = std::stoi(valStr);
            } else if (colType == DataType::FLOAT) {
                // Convert string to float and store it back in the variant
                row.values[i] = std::stof(valStr);
            } else if (colType == DataType::CHAR) {
                // Ensure the string is a single character
                if (valStr.length() != 1) {
                    throw std::runtime_error("Type mismatch: Expected single CHAR at column " + it->second.columns[i].name);
                }
                // Store the character in the variant
                row.values[i] = valStr[0];
            } else if (colType == DataType::DATE) {
                // Validate DATE format (YYYY-MM-DD)
                if (valStr.size() != 10 || valStr[4] != '-' || valStr[7] != '-' ||
                    !std::isdigit(valStr[0]) || !std::isdigit(valStr[1]) ||
                    !std::isdigit(valStr[2]) || !std::isdigit(valStr[3]) ||
                    !std::isdigit(valStr[5]) || !std::isdigit(valStr[6]) ||
                    !std::isdigit(valStr[8]) || !std::isdigit(valStr[9])) {
                    throw std::runtime_error("Type mismatch: Expected DATE in 'YYYY-MM-DD' format at column " + it->second.columns[i].name);
                    }
                // Store the date string (unchanged) in the variant
                row.values[i] = valStr;
            } else if (colType == DataType::VARCHAR) {
                // VARCHAR remains as a string
                row.values[i] = valStr;
            }
        } catch (const std::exception& e) {
            // Handle conversion errors
            throw std::runtime_error("Type mismatch: " + std::string(e.what()) + " at column " + it->second.columns[i].name);
        }
    }
    // Add the validated row to the table's list of rows
    it->second.rows.push_back(row);

    // Print a success message indicating the row was inserted
    fmt::print("Row inserted into '{}' successfully.\n", tableName);
}

// ------------------------------------------------------------------------------------------------------------------

void Database::selectFrom(const std::string& command) {
    // Remove trailing semicolon and trim the command
    std::string cleanedCommand = removeTrailingSemicolon(trim(command));

    // Find the positions of "FROM" and "WHERE" in the query
    std::size_t wherePos = cleanedCommand.find(" WHERE ");
    std::size_t fromPos = cleanedCommand.find(" FROM ");

    // Check if "FROM" is present; it is required
    if (fromPos == std::string::npos) {
        throw std::runtime_error("Syntax error in SELECT command.");
    }

    // Extract the part before "FROM" as column specifications
    std::string columnsPart = trim(cleanedCommand.substr(0, fromPos));

    // Extract the table name from the part after "FROM" but before "WHERE" (if present)
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

            // Find the index of the specified column
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

    // Calculate column widths
    for (size_t i = 0; i < colIndices.size(); ++i) {
        // Start with the column header's length
        colWidths[i] = table.columns[colIndices[i]].name.length();

        // Update width based on row data
        for (const auto& row : filteredRows) {
            const auto& value = row.values[colIndices[i]];
            size_t valueLength = 0;

            if (std::holds_alternative<int>(value)) {
                valueLength = std::to_string(std::get<int>(value)).length();
            } else if (std::holds_alternative<float>(value)) {
                // Add extra space for `.00` in float formatting
                valueLength = fmt::format("{:.2f}", std::get<float>(value)).length();
            } else if (std::holds_alternative<std::string>(value)) {
                valueLength = std::get<std::string>(value).length();
            }

            // Update the maximum width for the column
            colWidths[i] = std::max(colWidths[i], valueLength);
        }
    }

    // Adjust column header spacing for floats
    for (size_t i = 0; i < colIndices.size(); ++i) {
        if (table.columns[colIndices[i]].type == DataType::FLOAT) {
            // Add extra width for floating-point precision formatting
            colWidths[i] = std::max(colWidths[i], colWidths[i] + 3); // Adjust width for ".xx"
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
                // Integer value
                fmt::print(" {:<{}} |", std::get<int>(value), colWidths[i]);
            } else if (std::holds_alternative<float>(value)) {
                // Floating-point value (formatted to 2 decimal places)
                fmt::print(" {:<{}} |", fmt::format("{:.2f}", std::get<float>(value)), colWidths[i]);
            } else if (std::holds_alternative<std::string>(value)) {
                // String value
                fmt::print(" {:<{}} |", std::get<std::string>(value), colWidths[i]);
            }
        }
        fmt::print("\n");
    }
}

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