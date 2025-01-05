#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>

#include "database.h"
#include "utils.h"

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
        if (toCase(input, CaseType::UPPER) == "EXIT") {
            break;
        }

        // Display help if input is "HELP"
        if (toCase(input, CaseType::UPPER) == "HELP") {
            displayHelp();
            displayHeader();
            continue;
        }

        // Run tests if input is "TEST"
        if (toCase(input, CaseType::UPPER)  == "TEST") {
            runTests();
            displayHeader();
            continue;
        }

        // Split input by semicolons to handle multiple commands
        std::vector<std::string> commands = split(input, ';');

        for (auto& command : commands) {
            command = trim(command);

            // Skip empty commands
            if (command.empty()) {
                continue;
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

    fmt::print("\nExiting Database Application. Goodbye!\n");
    return 0;
}