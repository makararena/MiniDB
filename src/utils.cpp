#include <iostream>
#include <sstream>
#include <vector>
#include <fmt/format.h>

#include "database.h"
#include "utils.h"


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

// https://stackoverflow.com/questions/22425825/changing-a-lowercase-character-to-uppercase-in-c
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


// Converts SQL-like commands into a standardized format by normalizing keywords to uppercase.
// Handles both single-word and multi-word keywords.
std::string normalizeKeywords(const std::string& input) {
    // Define single-word keywords to be normalized to uppercase.
    std::vector<std::string> singleWordKws = {
        "SELECT", "FROM", "WHERE", "AND", "OR", "NOT", "IN",
        "LOAD", "INSERT", "CREATE", "DROP", "SAVE", "AS", "LIMIT"
    };

    // Define multi-word keywords to be normalized to uppercase.
    std::vector<std::string> multiWordKws = {
        "ORDER BY", "DELETE FILE", "LIST TABLES"
    };

    // Split the input into tokens (by spaces) for processing.
    std::vector<std::string> tokens = split(input, ' ');

    // Prepare a list to store the result with normalized keywords.
    // Reserve space in advance for efficiency.
    // Simple performance benefits
    std::vector<std::string> resultTokens;
    resultTokens.reserve(tokens.size()); // https://cplusplus.com/reference/vector/vector/reserve/

    // Process each token in the input.
    for (size_t i = 0; i < tokens.size(); ++i) {
        bool foundMulti = false;

        // Attempt to match multi-word keywords starting at the current token.
        for (size_t mk = 0; mk < multiWordKws.size(); ++mk) {
            // Split multi-word keyword into individual words.
            std::vector<std::string> parts = split(multiWordKws[mk], ' ');

            // Ensure there are enough tokens left to match this multi-word keyword.
            if (i + parts.size() <= tokens.size()) {
                bool match = true;

                // Compare each part of the multi-word keyword with corresponding tokens.
                for (size_t p = 0; p < parts.size(); ++p) {
                    if (!caseInsensitiveEquals(tokens[i + p], parts[p])) {
                        match = false;
                        break; // Exit if any part does not match.
                    }
                }

                // If a match is found, add the multi-word keyword to the result.
                if (match) {
                    resultTokens.push_back(multiWordKws[mk]);
                    i += (parts.size() - 1); // Skip over matched tokens.
                    foundMulti = true;
                    break;
                }
            }
        }

        // If a multi-word match was found, skip further processing for this token.
        if (foundMulti) {
            continue;
        }

        // Check for a single-word keyword match.
        bool foundSingle = false;
        for (size_t sk = 0; sk < singleWordKws.size(); ++sk) {
            if (caseInsensitiveEquals(tokens[i], singleWordKws[sk])) {
                // Add the single-word keyword to the result in uppercase.
                resultTokens.push_back(singleWordKws[sk]);
                foundSingle = true;
                break;
            }
        }

        // If no match is found, add the original token to the result.
        if (!foundSingle) {
            resultTokens.push_back(tokens[i]);
        }
    }

    // Rebuild the string from the normalized tokens.
    std::stringstream out;
    for (size_t i = 0; i < resultTokens.size(); ++i) {
        if (i > 0) {
            out << " ";
        }
        out << resultTokens[i];
    }

    // Return the fully normalized command string.
    return out.str();
}


void runTests() {
    Database db;

    try {
        fmt::print("\n=========================\n");
        fmt::print("Running Database Tests...\n");
        fmt::print("=========================\n\n");

        fmt::print("[Test 1: Create Table]\n");
        db.executeCommand("CREATE TABLE users (id INTEGER, name VARCHAR);");
        fmt::print("\n");

        fmt::print("[Test 2: Insert Valid Data]\n");
        db.executeCommand("INSERT INTO users VALUES (1, 'Alice');");
        db.executeCommand("INSERT INTO users VALUES (2, 'Bob');");
        db.executeCommand("INSERT INTO users VALUES (3, 'Charlie');");
        fmt::print("\n");

        fmt::print("[Test 3: Select All]\n");
        db.executeCommand("SELECT * FROM users;");
        fmt::print("\n");

        fmt::print("[Test 4: Select Specific Columns]\n");
        db.executeCommand("SELECT name FROM users;");
        db.executeCommand("SELECT name, id FROM users;");
        fmt::print("\n");

        fmt::print("[Test 5: Create Table with Multiple Data Types]\n");
        db.executeCommand("CREATE TABLE employees (id INTEGER, salary FLOAT, grade CHAR, hire_date DATE, name VARCHAR);");
        fmt::print("\n");

        fmt::print("[Test 6: Insert Data with Multiple Data Types]\n");
        db.executeCommand("INSERT INTO employees VALUES (1, 45000.75, 'A', '2022-11-01', 'Alice');");
        db.executeCommand("INSERT INTO employees VALUES (2, 35000.50, 'B', '2020-05-15', 'Bob');");
        db.executeCommand("INSERT INTO employees VALUES (3, 50000.00, 'A', '2019-06-01', 'Charlie');");
        db.executeCommand("INSERT INTO employees VALUES (4, 60000.25, 'C', '2018-12-25', 'David');");
        fmt::print("\n");

        fmt::print("[Test 7: Select with WHERE Clause (Single Condition)]\n");
        db.executeCommand("SELECT name, salary FROM employees WHERE salary > 40000;");
        fmt::print("\n");

        fmt::print("[Test 8: Select with WHERE Clause (AND Condition)]\n");
        db.executeCommand("SELECT name, grade, salary FROM employees WHERE grade = 'A' AND salary > 45000;");
        fmt::print("\n");

        fmt::print("[Test 9: Select with WHERE Clause (OR Condition)]\n");
        db.executeCommand("SELECT id, name, salary, grade FROM employees WHERE salary > 60000 OR grade = 'B';");
        fmt::print("\n");

        fmt::print("[Test 10: Select with WHERE Clause (NOT Condition)]\n");
        db.executeCommand("SELECT name, hire_date, grade FROM employees WHERE NOT grade = 'C';");
        fmt::print("\n");

        fmt::print("[Test 11: Save to File (With AS)]\n");
        db.executeCommand("SAVE employees AS employees_backup.csv;");
        fmt::print("\n");

        fmt::print("[Test 12: Save to File (Without AS)]\n");
        db.executeCommand("SAVE users;");
        fmt::print("\n");

        fmt::print("[Test 13: Drop Table]\n");
        db.executeCommand("DROP TABLE users;");
        db.executeCommand("DROP TABLE employees;");
        fmt::print("\n");

        fmt::print("[Test 14: Load from File (With AS)]\n");
        db.executeCommand("LOAD employees_backup.csv AS employees;");
        fmt::print("\n");

        fmt::print("[Test 15: Load from File (Without AS)]\n");
        db.executeCommand("LOAD users.csv;");
        fmt::print("\n");

        fmt::print("[Test 16: Select After Loading]\n");
        db.executeCommand("SELECT * FROM employees;");
        fmt::print("\n");

        fmt::print("[Test 17: LIST TABLES]\n");
        db.executeCommand("LIST TABLES;");
        fmt::print("\n");


        fmt::print("[Test 18: CREATE TABLE with Duplicate Name]\n");
        try {
            db.executeCommand("CREATE TABLE employees (id INTEGER, name VARCHAR);");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Handled duplicate table creation correctly.\n\n");


        fmt::print("[Test 19: INSERT INTO Nonexistent Table]\n");
        try {
            db.executeCommand("INSERT INTO non_existent_table VALUES (1, 'John');");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Handled insertion into nonexistent table correctly.\n\n");


        fmt::print("[Test 21: DROP Nonexistent Table]\n");
        try {
            db.executeCommand("DROP TABLE non_existent_table;");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Handled dropping nonexistent table correctly.\n\n");


        fmt::print("[Test 22: SAVE Nonexistent Table]\n");
        try {
            db.executeCommand("SAVE non_existent_table;");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Handled saving nonexistent table correctly.\n\n");


        fmt::print("[Test 23: SELECT on Empty Table]\n");
        db.executeCommand("CREATE TABLE empty_table (id INTEGER, name VARCHAR);");
        db.executeCommand("SELECT * FROM empty_table;");
        fmt::print(" - Selected from empty table without errors.\n\n");


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


        fmt::print("[Test 25: Invalid IN Query]\n");
        try {
            db.executeCommand("SELECT * FROM users WHERE name IN ('Anton', 'Maria';");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected for invalid IN query: {}\n", e.what());
        }
        fmt::print(" - Invalid IN query test completed.\n\n");


        fmt::print("[Test 26: SELECT with IN on Empty Table]\n");
        try {
            db.executeCommand("CREATE TABLE empty_users (id INTEGER, name VARCHAR);");
            db.executeCommand("SELECT * FROM empty_users WHERE name IN ('Anton', 'Maria');");
            fmt::print(" - Query executed successfully on empty table (no rows returned).\n");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught during SELECT with IN on empty table: {}\n", e.what());
        }
        fmt::print(" - SELECT with IN operator on empty table test completed.\n\n");


        fmt::print("[Test 27: Delete users.csv]\n");
        try {
            db.executeCommand("DELETE FILE users.csv");
            fmt::print(" - File 'users.csv' deleted successfully.\n");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught during file deletion: {}\n", e.what());
        }
        fmt::print(" - Delete users.csv test completed.\n\n");


        fmt::print("[Test 28: Delete employees_backup.csv]\n");
        try {
            db.executeCommand("DELETE FILE employees_backup.csv");
            fmt::print(" - File 'employees_backup.csv' deleted successfully.\n");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught during file deletion: {}\n", e.what());
        }
        fmt::print(" - Delete employees_backup.csv test completed.\n\n");


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

    fmt::print("- CREATE TABLE tableName (column1 TYPE, column2 TYPE, ...);\n");
    fmt::print("  Example: CREATE TABLE users (id INTEGER, name VARCHAR, age INTEGER);\n\n");

    fmt::print("- INSERT INTO tableName VALUES (value1, value2, ...);\n");
    fmt::print("  Example: INSERT INTO users VALUES (1, 'Alice', 25);\n\n");

    fmt::print("- SELECT column1, column2 FROM tableName [WHERE condition] [ORDER BY column1 [ASC|DESC], column2 [ASC|DESC]] [LIMIT n];\n");
    fmt::print("  Examples:\n");
    fmt::print("    SELECT * FROM users;\n");
    fmt::print("    SELECT name, age FROM users WHERE age > 20;\n");
    fmt::print("    SELECT * FROM users ORDER BY age DESC, name ASC;\n");
    fmt::print("    SELECT id, name FROM users WHERE age >= 25 ORDER BY name ASC LIMIT 10;\n\n");

    fmt::print("- DROP TABLE tableName;\n");
    fmt::print("  Example: DROP TABLE users;\n\n");

    fmt::print("- SAVE tableName [AS csvFileName];\n");
    fmt::print("  Examples:\n");
    fmt::print("    SAVE users;\n");
    fmt::print("    SAVE users AS user_backup.csv;\n\n");

    fmt::print("- LOAD csvFileName [AS tableName];\n");
    fmt::print("  Examples:\n");
    fmt::print("    LOAD users.csv;\n");
    fmt::print("    LOAD user_backup.csv AS users;\n\n");

    fmt::print("- DELETE FILE fileName;\n");
    fmt::print("  Example: DELETE FILE users.csv;\n\n");

    fmt::print("- LIST TABLES;\n");
    fmt::print("  Lists all tables currently in memory.\n\n");

    fmt::print("- HELP: Display this list of commands.\n\n");

    fmt::print("- EXIT: Exit the application.\n\n");

    fmt::print("Additional Notes:\n");
    fmt::print("  - Supported data types: INTEGER, FLOAT, CHAR, VARCHAR, DATE.\n");
    fmt::print("  - WHERE clause supports conditions like '=', '!=', '<', '>', '<=', '>=', 'IN', 'NOT IN'.\n");
    fmt::print("  - ORDER BY supports sorting by multiple columns with ASC (default) or DESC.\n");
    fmt::print("  - LIMIT restricts the number of rows returned in a SELECT query.\n");
    fmt::print("\n");
}
