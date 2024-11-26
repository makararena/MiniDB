// database.cpp

#include "database.h"
#include <sstream>
#include <fstream>
#include <algorithm>
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

    fmt::print("Executing command {}\n", operation);

    std::string restOfCommand;
    std::getline(ss, restOfCommand); // Extract everything after the first word
    restOfCommand = trim(restOfCommand);

    fmt::print("Executing the rest of the command {}\n", restOfCommand);

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
    columnsDef = trim(columnsDef);

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
    std::stringstream ss(command);
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
    valuesDef = trim(valuesDef);

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

    // Perform type checking to ensure each value matches the expected column type
    for (size_t i = 0; i < row.values.size(); ++i) {
        DataType colType = it->second.columns[i].type; // Get the expected type for the current column
        const std::string& valStr = row.values[i];     // Get the current value

        // Check if the value matches the column's data type
        if (colType == DataType::INTEGER) {
            try {
                // Attempt to convert the value to an integer
                std::stoi(valStr); // . Parses str interpreting its content as an integral number of the specified base, which is returned as an int value.
            } catch (...) {
                // If conversion fails, throw an error indicating a type mismatch
                throw std::runtime_error("Type mismatch: Expected INTEGER at column " + it->second.columns[i].name);
            }
        }
        // No need to check for STRING type explicitly, as all values are stored as strings
    }

    // Add the validated row to the table's list of rows
    it->second.rows.push_back(row);

    // Print a success message indicating the row was inserted
    fmt::print("Row inserted into '{}' successfully.\n", tableName);
}

// ------------------------------------------------------------------------------------------------------------------

void Database::selectFrom(const std::string& command) {
    // Expected formats:
    // "* FROM table_name;"
    // "column1, column2 FROM table_name;"

    std::stringstream ss(command);
    std::string columnsPart;
    ss >> columnsPart;

    std::string keyword;
    ss >> keyword;

    if (toUpperCase(keyword) != "FROM") {
        throw std::runtime_error("Syntax error in SELECT command.");
    }

    std::string tableName;
    ss >> tableName;

    auto it = tables.find(tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table '" + tableName + "' does not exist.");
    }

    Table& table = it->second;

    // Determine column indices
    std::vector<int> colIndices;
    bool selectAll = (columnsPart == "*");
    if (selectAll) {
        for (size_t i = 0; i < table.columns.size(); ++i) {
            colIndices.push_back(i);
        }
    } else {
        std::vector<std::string> selectedColumns = split(columnsPart, ',');
        for (auto& colName : selectedColumns) {
            colName = trim(colName);
            bool found = false;
            for (size_t i = 0; i < table.columns.size(); ++i) {
                if (table.columns[i].name == colName) {
                    colIndices.push_back(i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Column '" + colName + "' does not exist in table '" + tableName + "'.");
            }
        }
    }

    // Print column headers
    for (size_t idx : colIndices) {
        fmt::print("{}\t", table.columns[idx].name);
    }
    fmt::print("\n");

    // Print rows
    for (const auto& row : table.rows) {
        for (size_t idx : colIndices) {
            fmt::print("{}\t", row.values[idx]);
        }
        fmt::print("\n");
    }
}

void Database::saveToFile(const std::string& command) {
    // Expected format: "filename;"
    std::string filename = trim(command);
    if (filename.empty()) {
        throw std::runtime_error("Syntax error in SAVE command.");
    }

    std::ofstream ofs(filename);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for saving.");
    }

    // Iterate over all tables in the database
    for (const auto& pair : tables) {
        const Table& table = pair.second;

        // Save table name as a comment (optional)
        ofs << "# Table: " << table.name << "\n";

        // Save column headers
        for (size_t i = 0; i < table.columns.size(); ++i) {
            ofs << table.columns[i].name;
            if (i < table.columns.size() - 1) {
                ofs << ","; // Add a comma except for the last column
            }
        }
        ofs << "\n";

        // Save rows
        for (const auto& row : table.rows) {
            for (size_t i = 0; i < row.values.size(); ++i) {
                ofs << row.values[i];
                if (i < row.values.size() - 1) {
                    ofs << ","; // Add a comma except for the last value
                }
            }
            ofs << "\n";
        }

        // Add an empty line between tables for clarity
        ofs << "\n";
    }

    ofs.close();
    fmt::print("Database saved to '{}' in CSV format successfully.\n", filename);
}

void Database::loadFromFile(const std::string& command) {
    // Expected format: "filename;"
    std::string filename = trim(command);
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