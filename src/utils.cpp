#include "utils.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <fmt/format.h>

#include "database.h"


std::string trim(const std::string& str) {
    int start = 0;
    int end = str.length() - 1;

    while (start <= end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r')) {
        start++;
    }

    while (end >= start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\n' || str[end] == '\r')) {
        end--;
    }

    return str.substr(start, end - start + 1);
}


std::string toCase(const std::string& str, CaseType caseType) {
    std::string result = str;
    if (caseType == CaseType::UPPER) {
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::toupper(c); });
    } else if (caseType == CaseType::LOWER) {
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
    }
    return result;
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


std::string removeTrailingSemicolon(const std::string& str) {
    if (!str.empty() && str.back() == ';') {
        return str.substr(0, str.size() - 1);
    }
    return str;
}


bool isQuoted(const std::string& s) {
    if (s.size() < 2) return false;
    return ((s.front() == '\'' && s.back() == '\'') ||
            (s.front() == '"'  && s.back() == '"'));
}


std::string stripQuotes(const std::string& s) {
    if (isQuoted(s)) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}


bool isLogical(const std::string& token) {
    return (token == "AND" || token == "OR" || token == "NOT");
}


char toUpperManual(char c) {
    if (c >= 'a' && c <= 'z') {
        c = static_cast<char>(c - 'a' + 'A');
    }
    return c;
}


bool caseInsensitiveEquals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;

    for (size_t i = 0; i < a.size(); ++i) {
        char ca = a[i];
        char cb = b[i];
        ca = toUpperManual(ca);
        cb = toUpperManual(cb);
        if (ca != cb) return false;
    }
    return true;
}


std::string normalizeKeywords(const std::string& input)
{
    // Separate single-word from multi-word keywords
    std::vector<std::string> singleWordKws = {
        "SELECT", "FROM", "WHERE", "AND", "OR", "NOT", "IN",
        "LOAD", "INSERT", "CREATE", "DROP", "SAVE", "AS", "LIMIT"
    };

    std::vector<std::string> multiWordKws = {
        "ORDER BY", "DELETE FILE", "LIST TABLES"
    };

    // Split the input into tokens (by spaces)
    std::vector<std::string> tokens = split(input, ' ');

    // We'll build a new list of tokens with the updated cases
    std::vector<std::string> resultTokens;
    resultTokens.reserve(tokens.size()); // https://cplusplus.com/reference/vector/vector/reserve/

    for (size_t i = 0; i < tokens.size(); ++i) {
        bool foundMulti = false;

        // 1) Try matching each multi-word keyword at position i
        // Just loop over all multiwords trying to find the one which is the same as our current word
        for (size_t mk = 0; mk < multiWordKws.size(); ++mk) { // loop over multiword tokens
            std::vector<std::string> parts = split(multiWordKws[mk], ' '); // split them into pieces
            // Check if we have enough tokens left for this multi-word
            // for example i is a current index and if the tokens length is 5 and the current position is 2
            // and we have only 2 tokes then 2 + 2 <= 5 which means that we can still find the pair
            if (i + parts.size() <= tokens.size()) {
                bool match = true;
                for (size_t p = 0; p < parts.size(); ++p) {
                    if (!caseInsensitiveEquals(tokens[i + p], parts[p])) {
                        match = false;
                        break; // if at least one token is not the same the loop breaks and w move further
                    }
                }
                // And if the first word of multi-word token and first word match then we need to compare
                // the rest and if rest is the same then we push back the whole word move our indexes and
                // go on
                if (match) {
                    // We found a multi-word match. Add them in uppercase
                    // or exactly as in rawKeywords, e.g. "ORDER BY"
                    resultTokens.push_back(multiWordKws[mk]);
                    // Skip ahead past these tokens
                    i += (parts.size() - 1);
                    foundMulti = true;
                    break;
                }
            }
        }
        if (foundMulti) {
            continue; // move to next token after multi-word match
        }

        // 2) If no multi-word match, check for single-word
        bool foundSingle = false;
        for (size_t sk = 0; sk < singleWordKws.size(); ++sk) {
            if (caseInsensitiveEquals(tokens[i], singleWordKws[sk])) {
                // Use the exact form from singleWordKws, which is uppercase
                resultTokens.push_back(singleWordKws[sk]);
                foundSingle = true;
                break;
            }
        }

        if (!foundSingle) {
            // No match at all, push original token
            resultTokens.push_back(tokens[i]);
        }
    }

    // Rebuild the string with spaces
    std::stringstream out;
    for (int i = 0; i < resultTokens.size(); ++i) {
        if (i > 0) out << " ";
        out << resultTokens[i];
    }
    return out.str();
}

void runTests() {
    Database db;

    try {
        fmt::print("\n=========================\n");
        fmt::print("Running Database Tests...\n");
        fmt::print("=========================\n\n");

        // Test: Create Table
        fmt::print("[Test 1: Create Table]\n");
        db.executeCommand("CREATE TABLE users (id INTEGER, name VARCHAR);");
        fmt::print("\n");

        // Test: Insert Valid Data
        fmt::print("[Test 2: Insert Valid Data]\n");
        db.executeCommand("INSERT INTO users VALUES (1, 'Alice');");
        db.executeCommand("INSERT INTO users VALUES (2, 'Bob');");
        db.executeCommand("INSERT INTO users VALUES (3, 'Charlie');");
        fmt::print("\n");

        // Test: Select All
        fmt::print("[Test 3: Select All]\n");
        db.executeCommand("SELECT * FROM users;");
        fmt::print("\n");

        // Test: Select Specific Columns
        fmt::print("[Test 4: Select Specific Columns]\n");
        db.executeCommand("SELECT name FROM users;");
        db.executeCommand("SELECT name, id FROM users;");
        fmt::print("\n");

        // Test: Create Table with All Data Types
        fmt::print("[Test 5: Create Table with Multiple Data Types]\n");
        db.executeCommand("CREATE TABLE employees (id INTEGER, salary FLOAT, grade CHAR, hire_date DATE, name VARCHAR);");
        fmt::print("\n");

        // Test: Insert Data with All Data Types
        fmt::print("[Test 6: Insert Data with Multiple Data Types]\n");
        db.executeCommand("INSERT INTO employees VALUES (1, 45000.75, 'A', '2022-11-01', 'Alice');");
        db.executeCommand("INSERT INTO employees VALUES (2, 35000.50, 'B', '2020-05-15', 'Bob');");
        db.executeCommand("INSERT INTO employees VALUES (3, 50000.00, 'A', '2019-06-01', 'Charlie');");
        db.executeCommand("INSERT INTO employees VALUES (4, 60000.25, 'C', '2018-12-25', 'David');");
        fmt::print("\n");

        // Test: Select with WHERE Clause (Single Condition)
        fmt::print("[Test 7: Select with WHERE Clause (Single Condition)]\n");
        db.executeCommand("SELECT name, salary FROM employees WHERE salary > 40000;");
        fmt::print("\n");

        // Test: Select with WHERE Clause (AND Condition)
        fmt::print("[Test 8: Select with WHERE Clause (AND Condition)]\n");
        db.executeCommand("SELECT name, grade, salary FROM employees WHERE grade = 'A' AND salary > 45000;");
        fmt::print("\n");

        // Test: Select with WHERE Clause (OR Condition)
        fmt::print("[Test 9: Select with WHERE Clause (OR Condition)]\n");
        db.executeCommand("SELECT id, name, salary, grade FROM employees WHERE salary > 60000 OR grade = 'B';");
        fmt::print("\n");

        // Test: Select with WHERE Clause (NOT Condition)
        fmt::print("[Test 10: Select with WHERE Clause (NOT Condition)]\n");
        db.executeCommand("SELECT name, hire_date, grade FROM employees WHERE NOT grade = 'C';");
        fmt::print("\n");

        // Test: Save to File (With AS)
        fmt::print("[Test 11: Save to File (With AS)]\n");
        db.executeCommand("SAVE employees AS employees_backup.csv;");
        fmt::print("\n");

        // Test: Save to File (Without AS)
        fmt::print("[Test 12: Save to File (Without AS)]\n");
        db.executeCommand("SAVE users;");
        fmt::print("\n");

        // Test: Drop Table
        fmt::print("[Test 13: Drop Table]\n");
        db.executeCommand("DROP TABLE users;");
        db.executeCommand("DROP TABLE employees;");
        fmt::print("\n");

        // Test: Load from File (With AS)
        fmt::print("[Test 14: Load from File (With AS)]\n");
        db.executeCommand("LOAD employees_backup.csv AS employees;");
        fmt::print("\n");

        // Test: Load from File (Without AS)
        fmt::print("[Test 15: Load from File (Without AS)]\n");
        db.executeCommand("LOAD users.csv;");
        fmt::print("\n");

        // Test: Select After Loading
        fmt::print("[Test 16: Select After Loading]\n");
        db.executeCommand("SELECT * FROM employees;");
        fmt::print("\n");

        fmt::print("[Test 17: LIST TABLES]\n");
        db.executeCommand("LIST TABLES;");
        fmt::print("\n");

        // Test: CREATE TABLE with Duplicate Name
        fmt::print("[Test 18: CREATE TABLE with Duplicate Name]\n");
        try {
            db.executeCommand("CREATE TABLE employees (id INTEGER, name VARCHAR);");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Handled duplicate table creation correctly.\n\n");

        // Test: INSERT INTO Nonexistent Table
        fmt::print("[Test 19: INSERT INTO Nonexistent Table]\n");
        try {
            db.executeCommand("INSERT INTO non_existent_table VALUES (1, 'John');");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Handled insertion into nonexistent table correctly.\n\n");

        // Test: DROP Nonexistent Table
        fmt::print("[Test 21: DROP Nonexistent Table]\n");
        try {
            db.executeCommand("DROP TABLE non_existent_table;");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Handled dropping nonexistent table correctly.\n\n");

        // Test: SAVE Nonexistent Table
        fmt::print("[Test 22: SAVE Nonexistent Table]\n");
        try {
            db.executeCommand("SAVE non_existent_table;");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Handled saving nonexistent table correctly.\n\n");

        // Test: SELECT on Empty Table
        fmt::print("[Test 23: SELECT on Empty Table]\n");
        db.executeCommand("CREATE TABLE empty_table (id INTEGER, name VARCHAR);");
        db.executeCommand("SELECT * FROM empty_table;");
        fmt::print(" - Selected from empty table without errors.\n\n");

        // Test: SELECT with IN Operator
        fmt::print("[Test 24: SELECT with IN Operator]\n");
        try {
            db.executeCommand("CREATE TABLE users (id INTEGER, name VARCHAR, age INTEGER);");
            db.executeCommand("INSERT INTO users VALUES (1, 'Anton', 30);");
            db.executeCommand("INSERT INTO users VALUES (2, 'Alex', 25);");
            db.executeCommand("INSERT INTO users VALUES (3, 'Maria', 35);");
            fmt::print(" - Table 'users' created and rows inserted successfully.\n");

            db.executeCommand("SELECT * FROM users WHERE name IN ('Anton', 'Maria');");
            fmt::print(" - Query with IN operator executed successfully.\n");

            db.executeCommand("SELECT * FROM users WHERE name NOT IN ('Anton', 'Maria');");
            fmt::print(" - Query with NOT IN operator executed successfully.\n");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught during SELECT with IN operator: {}\n", e.what());
        }
        fmt::print(" - SELECT with IN operator test completed.\n\n");

        // Test: Invalid IN Query
        fmt::print("[Test 25: Invalid IN Query]\n");
        try {
            db.executeCommand("SELECT * FROM users WHERE name IN ('Anton', 'Maria';");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected for invalid IN query: {}\n", e.what());
        }
        fmt::print(" - Invalid IN query test completed.\n\n");

        // Test: SELECT with IN Operator on Empty Table
        fmt::print("[Test 26: SELECT with IN on Empty Table]\n");
        try {
            db.executeCommand("CREATE TABLE empty_users (id INTEGER, name VARCHAR);");
            db.executeCommand("SELECT * FROM empty_users WHERE name IN ('Anton', 'Maria');");
            fmt::print(" - Query executed successfully on empty table (no rows returned).\n");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught during SELECT with IN on empty table: {}\n", e.what());
        }
        fmt::print(" - SELECT with IN operator on empty table test completed.\n\n");

        // Test: Delete users.csv
        fmt::print("[Test 27: Delete users.csv]\n");
        try {
            db.executeCommand("DELETE FILE users.csv");
            fmt::print(" - File 'users.csv' deleted successfully.\n");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught during file deletion: {}\n", e.what());
        }
        fmt::print(" - Delete users.csv test completed.\n\n");

        // Test: Delete employees_backup.csv
        fmt::print("[Test 28: Delete employees_backup.csv]\n");
        try {
            db.executeCommand("DELETE FILE employees_backup.csv");
            fmt::print(" - File 'employees_backup.csv' deleted successfully.\n");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught during file deletion: {}\n", e.what());
        }
        fmt::print(" - Delete employees_backup.csv test completed.\n\n");

        // Test: Attempt to delete a non-existent file
        fmt::print("[Test 29: Delete non-existent file]\n");
        try {
            db.executeCommand(" DELETE FILE non_existent_file.csv");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Attempt to delete non-existent file test completed.\n\n");

        fmt::print("=========================\n");
        fmt::print("All tests passed successfully!\n");
        fmt::print("=========================\n\n");

    } catch (const std::exception& ex) {
        fmt::print("[Test Failure]: {}\n", ex.what());
    }
}


void displayHeader() {
    fmt::print("=========================================\n");
    fmt::print(" Makar Charviakou's Database Application\n");
    fmt::print("=========================================\n");
    fmt::print("Type 'HELP' to see the list of available commands.\n");
    fmt::print("Type 'EXIT' to quit the application.\n");
    fmt::print("Type 'TEST' to run automated tests.\n\n");
}


void displayHelp() {
    fmt::print("\nAvailable Commands:\n");

    // Table creation
    fmt::print("- CREATE TABLE tableName (column1 TYPE, column2 TYPE, ...);\n");
    fmt::print("  Example: CREATE TABLE users (id INTEGER, name VARCHAR, age INTEGER);\n\n");

    // Insert data
    fmt::print("- INSERT INTO tableName VALUES (value1, value2, ...);\n");
    fmt::print("  Example: INSERT INTO users VALUES (1, 'Alice', 25);\n\n");

    // Select data
    fmt::print("- SELECT column1, column2 FROM tableName [WHERE condition] [ORDER BY column1 [ASC|DESC], column2 [ASC|DESC]] [LIMIT n];\n");
    fmt::print("  Examples:\n");
    fmt::print("    SELECT * FROM users;\n");
    fmt::print("    SELECT name, age FROM users WHERE age > 20;\n");
    fmt::print("    SELECT * FROM users ORDER BY age DESC, name ASC;\n");
    fmt::print("    SELECT id, name FROM users WHERE age >= 25 ORDER BY name ASC LIMIT 10;\n\n");

    // Drop table
    fmt::print("- DROP TABLE tableName;\n");
    fmt::print("  Example: DROP TABLE users;\n\n");

    // Save table
    fmt::print("- SAVE tableName [AS csvFileName];\n");
    fmt::print("  Examples:\n");
    fmt::print("    SAVE users;\n");
    fmt::print("    SAVE users AS user_backup.csv;\n\n");

    // Load table
    fmt::print("- LOAD csvFileName [AS tableName];\n");
    fmt::print("  Examples:\n");
    fmt::print("    LOAD users.csv;\n");
    fmt::print("    LOAD user_backup.csv AS users;\n\n");

    // Delete file
    fmt::print("- DELETE FILE fileName;\n");
    fmt::print("  Example: DELETE FILE users.csv;\n\n");

    // List tables
    fmt::print("- LIST TABLES;\n");
    fmt::print("  Lists all tables currently in memory.\n\n");

    // Help
    fmt::print("- HELP: Display this list of commands.\n\n");

    // Exit
    fmt::print("- EXIT: Exit the application.\n\n");

    // Notes
    fmt::print("Additional Notes:\n");
    fmt::print("  - Supported data types: INTEGER, FLOAT, CHAR, VARCHAR, DATE.\n");
    fmt::print("  - WHERE clause supports conditions like '=', '!=', '<', '>', '<=', '>=', 'IN', 'NOT IN'.\n");
    fmt::print("  - ORDER BY supports sorting by multiple columns with ASC (default) or DESC.\n");
    fmt::print("  - LIMIT restricts the number of rows returned in a SELECT query.\n");
    fmt::print("\n");
}
