# MiniDB

A simple database implementation written in **C++**, created for educational purposes. This project showcases how to build a basic file-based database with functionalities like CRUD operations (Create, Read, Update, Delete) and demonstrates modular programming with `database.cpp` and `database.h`.

---

## Project Overview

This project simulates a minimalistic database system. It supports basic operations you might find in small-scale database systems, such as:

- Adding new entries to the database.
- Retrieving data.
- Updating specific records.
- Deleting records.

The code follows a clean structure to ensure maintainability and reusability, making it a perfect starting point for anyone learning how databases operate under the hood.

---

## Features

- **Command-Line Interface (CLI):** Interact with the database using commands.
- **File-Based Storage:** Data persists across sessions by saving it to files.
- **CRUD Functionality:** Core database operations are implemented.
- **Easy Extensibility:** Modular structure with separate `.cpp` and `.h` files for the database logic.

---

## Project Structure

```
Database-in-C/
│
├── .idea/                (IDE configuration - optional, can be gitignored)
├── cmake-build-debug/    (Build output - optional, can be gitignored)
├── src/
│   ├── database.cpp      (Core database implementation)
│   ├── database.h        (Header file for database.cpp)
│   ├── main.cpp          (Entry point for the application)
│
├── CMakeLists.txt        (Build configuration for CMake)
├── README.md             (This file)
├── Project Rules.txt     (Development rules or additional notes)
├── what_i_need_to_add.txt (A to-do list for future features)
```

---

## Setup and Installation

### Prerequisites
- **C++ Compiler:** Ensure you have a C++ compiler installed (e.g., `g++`, `clang`).
- **CMake:** Required for building the project.
- **Make:** Used to compile the project.

### Building the Project
1. Clone the repository:
   ```bash
   git clone https://github.com/makararena/Database-in-C.git
   cd Database-in-C
   ```

2. Create a `build/` directory and navigate to it:
   ```bash
   mkdir build && cd build
   ```

3. Generate build files using CMake:
   ```bash
   cmake ..
   ```

4. Compile the project:
   ```bash
   make
   ```

5. Run the executable:
   ```bash
   ./database
   ```

---

## Usage

### Command-Line Instructions
1. Once the program is running, you'll be presented with a menu of options.
2. Follow the prompts to perform the following operations:
   - Add a record to the database.
   - View all records.
   - Search for specific entries.
   - Update or delete existing entries.

### Example
```bash
Welcome to Database-in-C!
1. Add Record
2. View Records
3. Search Records
4. Update Record
5. Delete Record
6. Exit
Enter your choice: 1
```

---

## Future Features (Planned)

The project is designed to be extendable. Future updates may include:
- **Indexing:** Implementing indexes for faster search operations.
- **Transactions:** Adding support for simple database transactions.
- **Multi-File Database:** Expanding the program to work with multiple database files.
- **Tests:** Incorporating unit tests for database functionality.

---

## Contributions

If you'd like to contribute:
1. Fork the repository.
2. Create a feature branch.
3. Submit a pull request with detailed explanations of your changes.

---

## Notes (Optional - Content Above `main()`)

If your project contains introductory information in `main.cpp`, it's a great idea to move all that into this section or above, such as:
- A program introduction.
- Instructions to use the CLI.
- An explanation of how the project functions at a high level.

---

## License

This project is distributed under the **MIT License**. Feel free to use it as you see fit, with attribution.

---

This expanded README should provide a more comprehensive view of your project and its goals, making it easier for others to understand and contribute. Let me know if you'd like to refine any section further!
