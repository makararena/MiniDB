# **Database Application**

This is a lightweight and versatile database application written in C++. It allows users to create, manage, and query in-memory relational database tables with SQL-like commands. The application also supports saving and loading tables to and from files for persistence.

---
### Core Features
1. **Table Management**
   - Create and drop tables with customizable column definitions.
   - List all active tables.

2. **Data Manipulation**
   - Insert rows into tables with support for multiple data types:
     - `INTEGER`
     - `VARCHAR`
     - `DATE`
     - `CHAR`
     - `FLOAT`

3. **Query Execution**
   - `SELECT` command with features like:
     - Column selection (`SELECT column_name`)
     - Filtering (`WHERE` clause with support for `=`, `!=`, `<`, `>`, `<=`, `>=`, `IN`, `NOT IN`).
     - Logical operators (`AND`, `OR`, `NOT`).
     - Sorting (`ORDER BY` with `ASC` or `DESC`).
     - Limiting rows (`LIMIT`).

4. **Persistence**
   - Save tables to `.csv` files with `SAVE table_name [AS file_name]`.
   - Load tables from `.csv` files with `LOAD file_name [AS table_name]`.
   - Delete saved `.csv` files with `DELETE FILE file_name`.

5. **Utility Commands**
   - Display helpful instructions (`HELP`).
   - Run built-in tests (`TEST`) to validate functionality.

6. **Error Handling**
   - Detect syntax errors for commands like `CREATE TABLE`, `INSERT INTO`, and `SELECT`.
   - Handle invalid data types or mismatched columns during insertion.
   - Provide meaningful error messages for unsupported commands or operations.
---

## **Getting Started**

### **Building and Running with Docker**

To simplify setup, you can use Docker to build and run the application.

1. **Build the Docker Image**:
   ```bash
   docker build -t database-app .
   ```

2. **Run the Application**:
   ```bash
   docker run -it database-app
   ```

The application will start, and you can interact with it using the command-line interface.

---

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
- Test the application:
  ```
  TEST;
  ```

## **Contributing**

Feel free to fork this repository and make contributions. Pull requests are welcome for enhancements and bug fixes.

---

## **License**

This project is licensed under the MIT License. See the LICENSE file for details.