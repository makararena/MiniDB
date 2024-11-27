// database.hpp

// https://www.youtube.com/watch?v=RU5JUHAiR18
#ifndef DATABASE_HPP // Check if DATABASE_HPP is not defined
#define DATABASE_HPP // Define DATABASE_HPP

#include <string>
#include <vector>
#include <map>
#include <fmt/format.h>

// Enum to represent supported data types
enum class DataType {
    INTEGER,
    VARCHAR,
    DATE,    // New type
    CHAR,    // New type
    FLOAT    // New type
};

// Structure to represent a column in a table
struct Column {
    std::string name;
    DataType type;
};

// Class to represent a row in a table
using Value = std::variant<int, float, char, std::string>;

// Formatter specialization for std::variant<int, std::string>
template <>
struct fmt::formatter<Value> {
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Value& var, FormatContext& ctx) const -> decltype(ctx.out()) {
        if (std::holds_alternative<int>(var)) {
            return fmt::format_to(ctx.out(), "{}", std::get<int>(var));
        } else if (std::holds_alternative<float>(var)) {
            return fmt::format_to(ctx.out(), "{:.2f}", std::get<float>(var)); // Format floats with 2 decimal places
        } else if (std::holds_alternative<char>(var)) {
            return fmt::format_to(ctx.out(), "{}", std::get<char>(var)); // Format single characters
        } else if (std::holds_alternative<std::string>(var)) {
            return fmt::format_to(ctx.out(), "{}", std::get<std::string>(var)); // Handle VARCHAR and DATE
        }
        return ctx.out();
    }
};

struct Row {
    std::vector<Value> values;
};

// Class to represent a table in the database
struct Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;
};

struct Condition {
    std::string column;
    std::string op;     // Operator: =, >, <, etc.
    std::string value;  // Value to compare
    bool negate;        // True if NOT operator is used
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
std::string removeTrailingSemicolon(const std::string& str);
Condition parseCondition(const std::string& condStr);
std::vector<std::pair<std::string, Condition>> parseWhereClause(const std::string& wherePart);
bool evaluateCondition(const Row& row, const Table& table, const Condition& cond);
std::vector<Row> filterRows(const Table& table, const std::vector<std::pair<std::string, Condition>>& conditions);

// stop the definiton of the engine
#endif // DATABASE_HPP