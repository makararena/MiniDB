#pragma once
#include <string>
#include <vector>
#include "database.h"

struct Condition {
    std::string column;
    std::string op;
    std::string value;
    bool negate;
};

std::vector<std::pair<std::string, Condition>> parseWhereClause(const std::string& wherePart);
bool evaluateCondition(const Row& row, const Table& table, const Condition& cond);
std::vector<Row> filterRows(const Table& table, const std::vector<std::pair<std::string, Condition>>& conditions);
