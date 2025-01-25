#include <filesystem> // C++17 for filesystem
#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>

#include "database.h"
#include "file_io.h"
#include "utils.h"

// Function to clear terminal and reprint header
void clearScreenAndReset() {
    std::system("clear"); // Clear the terminal screen
    fmt::print("=========================================\n");
    fmt::print(" Makar Charviakou's Database Application \n");
    fmt::print("=========================================\n");
    fmt::print("Type 'HELP' to see the list of available commands.\n");
    fmt::print("Type 'EXIT' to quit the application.\n");
    fmt::print("Type 'TEST' to run automated tests.\n");
    fmt::print("Type 'CLEAR' to clear the screen and reset the view.\n");
    fmt::print("Type 'DATASETS' to list available datasets.\n");
}

void listDatasets(const std::string& folderPath) {
    fmt::print("\nLooking for datasets in folder '{}'\n", folderPath); // Debugging the folder path

    try {
        // Check if the directory exists, if not, create it
        if (!std::filesystem::exists(folderPath)) {
            std::filesystem::create_directories(folderPath);
        }

        fmt::print("Available datasets:\n\n");
        bool foundFile = false; // To handle the case where no files are found

        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) { // Ensure it's a file
                fmt::print("- {}\n", entry.path().filename().string());
                foundFile = true;
            }
        }

        if (!foundFile) {
            fmt::print("No datasets found in the folder '{}'.\n", folderPath);
        }
    } catch (const std::exception& e) {
        fmt::print("Error accessing datasets: {}\n", e.what());
    }
    fmt::print("\n");
}



int main() {
    Database db;
    std::string input;
    clearScreenAndReset();  // Initialize the header on startup

    while (true) {
        fmt::print("\n> "); // Always start the prompt on a new line
        std::getline(std::cin, input); // Get the input line

        if (toCase(input, CaseType::UPPER) == "EXIT") {
            break;
        }

        if (toCase(input, CaseType::UPPER) == "HELP") {
            fmt::print("\n"); // Add space before response
            fmt::print("Displaying help text...\n");
            displayHelp();
            fmt::print("\n"); // Add space after response
            continue;
        }

        if (toCase(input, CaseType::UPPER) == "TEST") {
            fmt::print("\n"); // Add space before response
            fmt::print("Running tests...\n");
            runTests();
            fmt::print("\n"); // Add space after response
            continue;
        }

        if (toCase(input, CaseType::UPPER) == "CLEAR") {
            clearScreenAndReset();
            continue;
        }

        if (toCase(input, CaseType::UPPER) == "DATASETS") {
            listDatasets(DATA_FOLDER); // Path to the data folder
            continue;
        }

        std::vector<std::string> commands = split(input, ';');

        for (auto& command : commands) {
            command = trim(command);

            // Skip empty commands
            if (command.empty()) {
                continue;
            }

            try {
                fmt::print("\n"); // Add space before response
                db.executeCommand(command);
                fmt::print("\n"); // Add space after response
            } catch (const std::exception& e) {
                fmt::print("\n"); // Add space before error message
                fmt::print("Error: {}\n", e.what());
                fmt::print("\n"); // Add space after error message
            } catch (...) {
                fmt::print("\n"); // Add space before unexpected error message
                fmt::print("An unexpected error occurred.\n");
                fmt::print("\n"); // Add space after unexpected error message
            }
        }
    }

    fmt::print("\nExiting Database Application. Goodbye!\n");
    return 0;
}
