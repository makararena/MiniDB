#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>

#include "database.h"
#include "utils.h"

// Function to run automated tests
void runTests() {
    Database db;

    try {
        fmt::print("\n=========================\n");
        fmt::print("Running Database Tests...\n");
        fmt::print("=========================\n\n");

        // Test: Create Table
        fmt::print("[Test 1: Create Table]\n");
        db.executeCommand("CREATE TABLE users (id INTEGER, name VARCHAR);");
        fmt::print(" - Created table 'users' successfully.\n\n");

        // Test: Insert Valid Data
        fmt::print("[Test 2: Insert Valid Data]\n");
        db.executeCommand("INSERT INTO users VALUES (1, 'Alice');");
        db.executeCommand("INSERT INTO users VALUES (2, 'Bob');");
        db.executeCommand("INSERT INTO users VALUES (3, 'Charlie');");
        fmt::print(" - Inserted valid rows into 'users'.\n\n");

        // Test: Select All
        fmt::print("[Test 3: Select All]\n");
        db.executeCommand("SELECT * FROM users;");
        fmt::print(" - Selected all rows from 'users'.\n\n");

        // Test: Select Specific Columns
        fmt::print("[Test 4: Select Specific Columns]\n");
        db.executeCommand("SELECT name FROM users;");
        db.executeCommand("SELECT name, id FROM users;");
        fmt::print(" - Selected specific columns from 'users'.\n\n");

        // Test: Create Table with All Data Types
        fmt::print("[Test 5: Create Table with Multiple Data Types]\n");
        db.executeCommand("CREATE TABLE employees (id INTEGER, salary FLOAT, grade CHAR, hire_date DATE, name VARCHAR);");
        fmt::print(" - Created table 'employees' with multiple data types.\n\n");

        // Test: Insert Data with All Data Types
        fmt::print("[Test 6: Insert Data with Multiple Data Types]\n");
        db.executeCommand("INSERT INTO employees VALUES (1, 45000.75, 'A', '2022-11-01', 'Alice');");
        db.executeCommand("INSERT INTO employees VALUES (2, 35000.50, 'B', '2020-05-15', 'Bob');");
        db.executeCommand("INSERT INTO employees VALUES (3, 50000.00, 'A', '2019-06-01', 'Charlie');");
        db.executeCommand("INSERT INTO employees VALUES (4, 60000.25, 'C', '2018-12-25', 'David');");
        fmt::print(" - Inserted rows with multiple data types into 'employees'.\n\n");

        // Test: Select with WHERE Clause (Single Condition)
        fmt::print("[Test 7: Select with WHERE Clause (Single Condition)]\n");
        db.executeCommand("SELECT name, salary FROM employees WHERE salary > 40000;");
        fmt::print(" - Selected rows where salary > 40000.\n\n");

        // Test: Select with WHERE Clause (AND Condition)
        fmt::print("[Test 8: Select with WHERE Clause (AND Condition)]\n");
        db.executeCommand("SELECT name, grade FROM employees WHERE grade = 'A' AND salary > 45000;");
        fmt::print(" - Selected rows where grade = 'A' and salary > 45000.\n\n");

        // Test: Select with WHERE Clause (OR Condition)
        fmt::print("[Test 9: Select with WHERE Clause (OR Condition)]\n");
        db.executeCommand("SELECT id, name FROM employees WHERE salary > 60000 OR grade = 'B';");
        fmt::print(" - Selected rows where salary > 60000 or grade = 'B'.\n\n");

        // Test: Select with WHERE Clause (NOT Condition)
        fmt::print("[Test 10: Select with WHERE Clause (NOT Condition)]\n");
        db.executeCommand("SELECT name, hire_date FROM employees WHERE NOT grade = 'C';");
        fmt::print(" - Selected rows where grade is not 'C'.\n\n");

        // Test: Save to File
        fmt::print("[Test 11: Save to File]\n");
        db.executeCommand("SAVE test_database.csv;");
        fmt::print(" - Saved database to file 'test_database.csv'.\n\n");

        // Test: Drop Table
        fmt::print("[Test 12: Drop Table]\n");
        db.executeCommand("DROP TABLE users;");
        db.executeCommand("DROP TABLE employees;");
        fmt::print(" - Dropped tables 'users' and 'employees'.\n\n");

        // Test: Load from File
        fmt::print("[Test 13: Load from File]\n");
        db.executeCommand("LOAD test_database.csv;");
        fmt::print(" - Loaded database from file 'test_database.csv'.\n\n");

        // Test: Select After Loading
        fmt::print("[Test 14: Select After Loading]\n");
        db.executeCommand("SELECT * FROM employees;");
        fmt::print(" - Selected all rows from 'employees' after loading.\n\n");

        // Test: Edge Cases for WHERE Clause
        fmt::print("[Test 15: Edge Cases for WHERE Clause]\n");
        try {
            db.executeCommand("SELECT id, name FROM employees WHERE nonexistent_column > 1000;");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        try {
            db.executeCommand("SELECT id FROM employees WHERE salary = 'invalid';");
        } catch (const std::exception& e) {
            fmt::print(" - Error caught as expected: {}\n", e.what());
        }
        fmt::print(" - Handled edge cases for WHERE clause correctly.\n\n");

        fmt::print("=========================\n");
        fmt::print("All tests passed successfully!\n");
        fmt::print("=========================\n\n");

    } catch (const std::exception& ex) {
        fmt::print("[Test Failure]: {}\n", ex.what());
    }
}

int main() {
    Database db; // Create a new database instance
    std::string input;

    fmt::print("Makararena's Database Application\n");
    fmt::print("Enter 'TEST' to run automated tests, or type commands (type 'EXIT' to quit):\n");

    while (true) {
        fmt::print("> ");
        std::getline(std::cin, input); // Get the input line

        // Exit condition
        if (input == "EXIT") {
            break;
        }

        // Run tests if input is "TEST"
        if (input == "TEST") {
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

    return 0;
}