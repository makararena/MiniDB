// database.hpp

// https://www.youtube.com/watch?v=RU5JUHAiR18
#ifndef DATABASE_HPP // Check if DATABASE_HPP is not defined
#define DATABASE_HPP // Define DATABASE_HPP

#include <string>
#include <vector>
#include <map>

// Enum to represent supported data types
enum class DataType {
    INTEGER,
    VARCHAR
};

// Structure to represent a column in a table
struct Column {
    std::string name;
    DataType type;
};

// Class to represent a row in a table
struct  Row {
    std::vector<std::string> values; // Store values as strings for simplicity
};

// Class to represent a table in the database
struct Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;
};

// Main Database class
class Database {
private:
    std::map<std::string, Table> tables;

    // Helper methods for command execution
    void createTable(const std::string& command);
    void dropTable(const std::string& command);
    void insertInto(const std::string& command);
    void selectFrom(const std::string& command);
    void saveToFile(const std::string& command);
    void loadFromFile(const std::string& command);

    // Helper methods
    DataType parseDataType(const std::string& typeStr);

public:
    Database();

    // Method to execute a given command
    void executeCommand(const std::string& command);
};

// Helper functions accessible in both main.cpp and database.cpp
std::string trim(const std::string& str);
std::string toUpperCase(const std::string& str);
std::vector<std::string> split(const std::string& s, char delimiter);

// stop the definiton of the engine
#endif // DATABASE_HPP