#include "condition.h"
#include "database.h"
#include "utils.h"

std::vector<std::pair<std::string, Condition>> parseWhereClause(const std::string& wherePart) {
    std::vector<std::pair<std::string, Condition>> conditions; // Resulting list of parsed conditions
    std::vector<std::string> tokens = split(wherePart, ' ');   // Split the `WHERE` clause into tokens by spaces

    std::string logicalOp; // Current logical operator between conditions (e.g., AND, OR, NOT)
    std::string currentColumn, currentOp, currentValue; // Components of a condition
    bool inCondition = false; // Indicates if we are building a condition
    bool inValue = false;     // Indicates if we are processing a multi-word value
    std::string valueBuffer;  // Temporary storage for multi-word string values

    // Iterate through each token in the WHERE clause
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::string token = tokens[i];

        // Check if the token is a logical operator
        if (token == "AND" || token == "OR" || token == "NOT") {
            if (inCondition) {
                // Finalize the current condition before moving to the next
                Condition cond{currentColumn, currentOp, currentValue, (logicalOp == "NOT")};
                conditions.emplace_back(logicalOp, cond); // Add the condition and logical operator to the list
                inCondition = false; // Reset for the next condition
            }
            logicalOp = token; // Update the current logical operator
        } else {
            // If it's not a logical operator, build a condition
            if (!inCondition) {
                // Start a new condition
                inCondition = true;
                currentColumn = token; // First token is the column name
            } else if (currentOp.empty()) {
                // Second token is the operator (e.g., =, >, <)
                currentOp = token;
            } else {
                // Remaining tokens are part of the value
                if (token.front() == '\'' || inValue) {
                    // If the value starts with a single quote or we are in a multi-word value
                    inValue = true; // Mark as inside a multi-word value
                    valueBuffer += (valueBuffer.empty() ? "" : " ") + token; // Append token to the value buffer
                    if (token.back() == '\'') {
                        // If the value ends with a single quote, finalize the value
                        currentValue = valueBuffer.substr(1, valueBuffer.size() - 2); // Strip the surrounding quotes
                        valueBuffer.clear(); // Clear the buffer
                        inValue = false;     // Exit multi-word value mode
                    }
                } else {
                    // If the value is a simple token (not quoted)
                    currentValue = token;
                }
            }
        }
    }

    // Finalize the last condition if any
    if (inCondition) {
        Condition cond{currentColumn, currentOp, currentValue, (logicalOp == "NOT")};
        conditions.emplace_back(logicalOp, cond); // Add the final condition
    }

    return conditions; // Return the list of parsed conditions
}

// Helper: Evaluate a condition for a single row
bool evaluateCondition(const Row& row, const Table& table, const Condition& cond) {
    auto colIt = std::find_if(
        table.columns.begin(),
        table.columns.end(),
        [&](const Column& col) { return col.name == cond.column; }
    );

    if (colIt == table.columns.end()) {
        throw std::runtime_error("Column '" + cond.column + "' does not exist.");
    }

    size_t colIndex = std::distance(table.columns.begin(), colIt);
    const Value& value = row.values[colIndex];

    bool result = false;
    try {
        if (std::holds_alternative<int>(value)) {
            int intValue = std::get<int>(value);
            int condValue = std::stoi(cond.value);
            if (cond.op == "=") result = (intValue == condValue);
            else if (cond.op == ">") result = (intValue > condValue);
            else if (cond.op == "<") result = (intValue < condValue);
        } else if (std::holds_alternative<float>(value)) {
            float floatValue = std::get<float>(value);
            float condValue = std::stof(cond.value);
            if (cond.op == "=") result = (floatValue == condValue);
            else if (cond.op == ">") result = (floatValue > condValue);
            else if (cond.op == "<") result = (floatValue < condValue);
        } else if (std::holds_alternative<char>(value)) {
            char charValue = std::get<char>(value);
            if (cond.op == "=") result = (charValue == cond.value[0]);
        } else if (std::holds_alternative<std::string>(value)) {
            const std::string& strValue = std::get<std::string>(value);
            if (cond.op == "=") result = (strValue == cond.value);
        }
    } catch (...) {
        throw std::runtime_error("Type mismatch in WHERE clause: Cannot compare '" + cond.value + "' to column '" + cond.column + "'");
    }

    return cond.negate ? !result : result;
}

// Helper: Apply WHERE clause to rows
std::vector<Row> filterRows(const Table& table, const std::vector<std::pair<std::string, Condition>>& conditions) {
    std::vector<Row> filteredRows;

    for (const auto& row : table.rows) {
        bool overallResult = (conditions.empty() || conditions[0].first.empty()) ? true : false;

        for (const auto& [logicalOp, cond] : conditions) {
            bool condResult = evaluateCondition(row, table, cond);

            if (logicalOp == "AND") {
                overallResult = overallResult && condResult;
            } else if (logicalOp == "OR") {
                overallResult = overallResult || condResult;
            } else {
                overallResult = condResult; // First condition
            }
        }

        if (overallResult) {
            filteredRows.push_back(row);
        }
    }

    return filteredRows;
}