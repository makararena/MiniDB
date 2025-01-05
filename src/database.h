#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>

// Enum for supported data types
enum class DataType {
    INTEGER,
    VARCHAR,
    DATE,
    CHAR,
    FLOAT
};

// Represents a column in a table
struct Column {
    std::string name; // Column name
    DataType type;    // Column data type
};

// A single value in a row
// using Value - creating new type alias (https://stackoverflow.com/questions/20790932/what-is-the-logic-behind-the-using-keyword-in-c)
// std::variant - https://www.geeksforgeeks.org/std-variant-in-cpp-17/
using Value = std::variant<int, float, char, std::string>;

// Represents a single row of data
struct Row {
    std::vector<Value> values; // Values in the row
};

// Represents a table in the database
struct Table {
    std::string name;              // Table name
    std::vector<Column> columns;   // Column definitions
    std::vector<Row> rows;         // Rows of data
};

// Main Database class
class Database {
private:
    std::map<std::string, Table> tables; // Map of table names to Table objects

    // Private helpers
    void createTable(const std::string& command);
    void dropTable(const std::string& command);
    void insertInto(const std::string& command);
    void selectFrom(const std::string& command);
    void listTables();

    // File IO
    void saveToFile(const std::string& command);
    void loadFromFile(const std::string& command);
    static void deleteFile(const std::string& command);

    DataType parseDataType(const std::string& typeStr);

public:
    Database();
    // API for interacting with the database
    void executeCommand(const std::string& command);
};
