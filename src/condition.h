#pragma once
#include <string>
#include <vector>
#include "database.h"

struct Condition {
    std::string column;
    std::string op;             // e.g. =, >, <, >=, <=, !=, IN, etc.
    std::string value;          // single value if op != "IN"
    std::vector<std::string> inValues; // multiple values if op == "IN"
    bool negate;                // true if there's a preceding NOT
};

// This function is responsible for parsing a SQL-like WHERE clause into a structured format that can be used for filtering database rows.
std::vector<std::pair<std::string, Condition>> parseWhereClause(const std::string& wherePart);

// Checks if a row satisfies a given condition.
// Uses the column, operator, and value(s) from the condition.
// Supports various data types and handles NOT logic.
bool evaluateCondition(const Row& row, const Table& table, const Condition& cond);

// Filters rows in a table based on multiple conditions.
// Combines results using logical operators (AND, OR).
// Returns only rows that satisfy all conditions.
std::vector<Row> filterRows(const Table& table, const std::vector<std::pair<std::string, Condition>>& conditions);
