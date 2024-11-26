#include "database.h"
#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h> // is mostly used for input / output and ranges are mostly used for string manipulation

int main() {
    Database db; // Create a new database instance
    std::string input;

    fmt::print("Makararena's Database Application\n");
    fmt::print("Enter commands (type 'EXIT' to quit):\n");

    while (true) {
        fmt::print("> ");
        std::getline(std::cin, input); // here we are getting the line and rewriting the input

        // Exit condition
        if (input == "EXIT") {
            break;
        }

        // Split input by semicolons to handle multiple commands
        // it's basic to have this type of separator for SQL because ";" shows the end of the query
        std::vector<std::string> commands = split(input, ';');

        for (auto& command : commands) {
            // Trim the command
            command = trim(command);

            // Skip empty commands
            if (command.empty()) {
                continue;
            }

            // Process the command using the database's executeCommand method
            try {
                db.executeCommand(command);
            } catch (const std::exception& e) {
                // Catch and display any errors encountered during command execution
                fmt::print("Error: {}\n", e.what());
            } catch (...) {
                // Catch any other exceptions
                fmt::print("An unexpected error occurred.\n");
            }
        }
    }

    return 0;
}