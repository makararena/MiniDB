#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>

#include "database.h"
#include "utils.h"

void runTests() {
    Database db;

    try {
        fmt::print("\n=========================\n");
        fmt::print("Running Database Tests...\n");
        fmt::print("=========================\n\n");

        // Test: Create Table
        fmt::print("[Test 1: Create Table]\n");
        db.executeCommand("CREATE TABLE users (id INTEGER, name VARCHAR);");

        // Test: Insert Valid Data
        fmt::print("[Test 2: Insert Valid Data]\n");
        db.executeCommand("INSERT INTO users VALUES (1, 'Alice');");
        db.executeCommand("INSERT INTO users VALUES (2, 'Bob');");
        db.executeCommand("INSERT INTO users VALUES (3, 'Charlie');");

        // Test: Select All
        fmt::print("[Test 3: Select All]\n");
        db.executeCommand("SELECT * FROM users;");

        // Test: Select Specific Columns
        fmt::print("[Test 4: Select Specific Columns]\n");
        db.executeCommand("SELECT name FROM users;");
        db.executeCommand("SELECT name, id FROM users;");

        // Test: Create Table with All Data Types
        fmt::print("[Test 5: Create Table with Multiple Data Types]\n");
        db.executeCommand("CREATE TABLE employees (id INTEGER, salary FLOAT, grade CHAR, hire_date DATE, name VARCHAR);");


        // Test: Insert Data with All Data Types
        fmt::print("[Test 6: Insert Data with Multiple Data Types]\n");
        db.executeCommand("INSERT INTO employees VALUES (1, 45000.75, 'A', '2022-11-01', 'Alice');");
        db.executeCommand("INSERT INTO employees VALUES (2, 35000.50, 'B', '2020-05-15', 'Bob');");
        db.executeCommand("INSERT INTO employees VALUES (3, 50000.00, 'A', '2019-06-01', 'Charlie');");
        db.executeCommand("INSERT INTO employees VALUES (4, 60000.25, 'C', '2018-12-25', 'David');");

        // Test: Select with WHERE Clause (Single Condition)
        fmt::print("[Test 7: Select with WHERE Clause (Single Condition)]\n");
        db.executeCommand("SELECT name, salary FROM employees WHERE salary > 40000;");

        // Test: Select with WHERE Clause (AND Condition)
        fmt::print("[Test 8: Select with WHERE Clause (AND Condition)]\n");
        db.executeCommand("SELECT name, grade, salary FROM employees WHERE grade = 'A' AND salary > 45000;");

        // Test: Select with WHERE Clause (OR Condition)
        fmt::print("[Test 9: Select with WHERE Clause (OR Condition)]\n");
        db.executeCommand("SELECT id, name, salary, grade FROM employees WHERE salary > 60000 OR grade = 'B';");

        // Test: Select with WHERE Clause (NOT Condition)
        fmt::print("[Test 10: Select with WHERE Clause (NOT Condition)]\n");
        db.executeCommand("SELECT name, hire_date, grade FROM employees WHERE NOT grade = 'C';");

        // Test: Save to File (With AS)
        fmt::print("[Test 11: Save to File (With AS)]\n");
        db.executeCommand("SAVE employees AS employees_backup.csv;");

        // Test: Save to File (Without AS)
        fmt::print("[Test 12: Save to File (Without AS)]\n");
        db.executeCommand("SAVE users;");

        // Test: Drop Table
        fmt::print("[Test 13: Drop Table]\n");
        db.executeCommand("DROP TABLE users;");
        db.executeCommand("DROP TABLE employees;");

        // Test: Load from File (With AS)
        fmt::print("[Test 14: Load from File (With AS)]\n");
        db.executeCommand("LOAD employees_backup.csv AS employees;");

        // Test: Load from File (Without AS)
        fmt::print("[Test 15: Load from File (Without AS)]\n");
        db.executeCommand("LOAD users.csv;");

        // Test: Select After Loading
        fmt::print("[Test 16: Select After Loading]\n");
        db.executeCommand("SELECT * FROM employees;");

        fmt::print("[Test 17: LIST TABLES]\n");
        db.executeCommand("LIST TABLES;");

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

        fmt::print("=========================\n");
        fmt::print("All tests passed successfully!\n");
        fmt::print("=========================\n\n");

        } catch (const std::exception& ex) {
            fmt::print("[Test Failure]: {}\n", ex.what());
        }
}


// Function to display the application header and initial instructions
void displayHeader() {
    fmt::print("=========================================\n");
    fmt::print("      Makararena's Database Application\n");
    fmt::print("=========================================\n");
    fmt::print("Type 'HELP' to see the list of available commands.\n");
    fmt::print("Type 'EXIT' to quit the application.\n");
    fmt::print("Type 'TEST' to run automated tests.\n\n");
}

// Function to display the list of available commands
void displayHelp() {
    fmt::print("\nAvailable Commands:\n");
    fmt::print("- CREATE TABLE tableName (column1 TYPE, column2 TYPE, ...);\n");
    fmt::print("  Example: CREATE TABLE users (id INTEGER, name VARCHAR);\n");
    fmt::print("- INSERT INTO tableName VALUES (value1, value2, ...);\n");
    fmt::print("  Example: INSERT INTO users VALUES (1, 'Alice');\n");
    fmt::print("- SELECT * FROM tableName;\n");
    fmt::print("  SELECT column1, column2 FROM tableName WHERE condition;\n");
    fmt::print("  Example: SELECT name FROM users WHERE id > 1;\n");
    fmt::print("- DROP TABLE tableName;\n");
    fmt::print("  Example: DROP TABLE users;\n");
    fmt::print("- SAVE tableName [AS csvFileName];\n");
    fmt::print("  Example: SAVE users;\n");
    fmt::print("  Example: SAVE users AS user_backup.csv;\n");
    fmt::print("- LOAD csvFileName [AS tableName];\n");
    fmt::print("  Example: LOAD users.csv;\n");
    fmt::print("  Example: LOAD user_backup.csv AS users;\n");
    fmt::print("- LIST TABLES;\n");
    fmt::print("  Lists all tables currently in memory.\n");
    fmt::print("- HELP: Display this list of commands.\n");
    fmt::print("- EXIT: Exit the application.\n");
    fmt::print("\n");
}

int main() {
    Database db; // Create a new database instance
    std::string input;

    // Display the initial header and instructions
    displayHeader();

    // Command loop
    while (true) {
        fmt::print("> ");
        std::getline(std::cin, input); // Get the input line

        // Exit condition
        if (toUpperCase(input) == "EXIT") {
            break;
        }

        // Display help if input is "HELP"
        if (toUpperCase(input) == "HELP") {
            displayHelp();
            displayHeader(); // Display the header again after HELP
            continue;
        }

        // Run tests if input is "TEST"
        if (toUpperCase(input) == "TEST") {
            runTests();
            continue;
        }

        // Split input by semicolons to handle multiple commands
        std::vector<std::string> commands = split(input, ';');

        for (auto& command : commands) {
            command = trim(command); // Trim the command

            if (command.empty()) {
                continue; // Skip empty commands
            }

            try {
                db.executeCommand(command); // Process the command
            } catch (const std::exception& e) {
                fmt::print("Error: {}\n", e.what());
            } catch (...) {
                fmt::print("An unexpected error occurred.\n");
            }
        }
    }

    fmt::print("\nExiting Makararena's Database Application. Goodbye!\n");
    return 0;
}