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



std::vector<std::pair<std::string, Condition>> parseWhereClause(const std::string& wherePart);
bool evaluateCondition(const Row& row, const Table& table, const Condition& cond);
std::vector<Row> filterRows(const Table& table, const std::vector<std::pair<std::string, Condition>>& conditions);
