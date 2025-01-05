# Database Application

This is a lightweight and versatile database application written in C++. It allows users to create, manage, and query in-memory relational database tables with SQL-like commands. The application also supports saving and loading tables to and from files for persistence.

---

## **Features**

- **Table Management**: Create and drop tables with customizable column definitions.
- **Data Manipulation**: Insert rows into tables with support for multiple data types:
    - `INTEGER`
    - `VARCHAR`
    - `DATE`
    - `CHAR`
    - `FLOAT`
- **Query Execution**:
    - `SELECT` with column selection, filtering using `WHERE`, sorting using `ORDER BY`, and limiting rows using `LIMIT`.
    - Logical operators like `AND`, `OR`, `NOT`, and `IN` are supported.
- **Persistence**:
    - Save tables to `.csv` files and load them back.
    - Delete files directly from the application.
- **Utility Commands**:
    - List all active tables.
    - Display helpful command instructions.
    - Run built-in tests to verify functionality.

---

## **Getting Started**

### **Project Structure**

```
DatabaseProject
├── CMakeLists.txt          # CMake build configuration
├── README.md               # Project documentation
├── data/                   # Sample data files
│   ├── Titanic.csv
│   ├── WorldPopulation.csv
├── docs/                   # Documentation files
├── src/                    # Source code files
│   ├── condition.cpp
│   ├── condition.h
│   ├── database.cpp
│   ├── database.h
│   ├── file_io.cpp
│   ├── file_io.h
│   ├── main.cpp
│   ├── utils.cpp
│   ├── utils.h
├── cmake-build-debug/      # Build artifacts
```

### **Prerequisites**

- A C++17 or higher compatible compiler (e.g., GCC, Clang, MSVC).
- CMake for building the project.
- [fmt library](https://github.com/fmtlib/fmt) installed on your system.

### **Building the Project**

1. Clone the repository or download the source files.
2. Navigate to the project root directory.
3. Build the project using CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
4. The executable file will be generated in the `build` directory.

### **Running the Application**

1. Navigate to the `build` directory.
2. Run the executable:
   ```bash
   ./database_app
   ```

---

## **Usage Instructions**

When you start the application, a prompt (`>`) will appear, allowing you to enter commands. Below are some of the supported commands:

### **Table Commands**

- Create a table:

  ```
  CREATE TABLE tableName (column1 TYPE, column2 TYPE, ...);
  Example: CREATE TABLE users (id INTEGER, name VARCHAR, age INTEGER);
  ```

- Drop a table:

  ```
  DROP TABLE tableName;
  Example: DROP TABLE users;
  ```

### **Data Commands**

- Insert data:

  ```
  INSERT INTO tableName VALUES (value1, value2, ...);
  Example: INSERT INTO users VALUES (1, 'Alice', 25);
  ```

- Query data:

  ```
  SELECT column1, column2 FROM tableName [WHERE condition] [ORDER BY column1 [ASC|DESC], column2 [ASC|DESC]] [LIMIT n];
  Examples:
    SELECT * FROM users;
    SELECT name, age FROM users WHERE age > 20;
    SELECT * FROM users ORDER BY age DESC LIMIT 10;
  ```

### **Persistence Commands**

- Save a table:

  ```
  SAVE tableName [AS fileName];
  Examples:
    SAVE users;
    SAVE users AS users_backup.csv;
  ```

- Load a table:

  ```
  LOAD fileName [AS tableName];
  Examples:
    LOAD users.csv;
    LOAD users_backup.csv AS users;
  ```

- Delete a file:

  ```
  DELETE FILE fileName;
  Example: DELETE FILE users_backup.csv;
  ```

### **Utility Commands**

- List tables:
  ```
  LIST TABLES;
  ```
- Display help:
  ```
  HELP;
  ```
- Exit the application:
  ```
  EXIT;
  ```

---

## **Testing**

You can run built-in tests by typing the following command at the prompt:

```bash
TEST
```

The tests will validate the core functionality of the application, such as creating tables, inserting data, querying with conditions, and saving/loading tables.

---

## **Contributing**

Feel free to fork this repository and make contributions. Pull requests are welcome for enhancements and bug fixes.

---

## **License**

This project is licensed under the MIT License. See the LICENSE file for details.

