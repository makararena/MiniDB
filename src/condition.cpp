#include "condition.h"
#include "database.h"
#include "utils.h"

// Extended parse to handle multi-character operators
std::vector<std::pair<std::string, Condition>> parseWhereClause(const std::string& wherePart) {
    std::vector<std::pair<std::string, Condition>> conditions;
    // Naive split on spaces
    std::vector<std::string> tokens = split(wherePart, ' ');

    std::string logicalOp;         // e.g., AND, OR
    std::string currentColumn;
    std::string currentOp;
    std::string currentValue;
    bool inCondition = false;      // Are we building a condition?
    bool inQuote     = false;      // Are we building a multi-token string value?
    std::string valueBuffer;
    std::vector<std::string> inValueList;

    // If we saw "NOT" but haven't yet seen "IN", store it here
    bool nextConditionShouldNegate = false;

    // Helper to finalize a single condition
    auto finalizeCondition = [&](bool forceNegate = false) {
        Condition cond;
        cond.column = currentColumn;
        cond.op     = currentOp;
        cond.negate = (forceNegate || nextConditionShouldNegate);

        if (cond.op == "IN") {
            cond.inValues = inValueList;
        } else {
            cond.value = currentValue;
        }

        // Add the condition to the list
        conditions.emplace_back(logicalOp, cond);

        // Reset state
        currentColumn.clear();
        currentOp.clear();
        currentValue.clear();
        inValueList.clear();
        valueBuffer.clear();
        inCondition = false;
        inQuote     = false;
        nextConditionShouldNegate = false;
    };

    for (size_t i = 0; i < tokens.size(); ++i) {
        std::string token = tokens[i];
        if (token.empty()) continue;

        // 1) Check for logical operators
        if (isLogical(token)) {
            // If we see NOT, set the flag so if we see "IN" next, it's "NOT IN"
            if (token == "NOT") {
                nextConditionShouldNegate = true;
            }
            else {
                // It's AND or OR
                // If we are already building a condition, finalize it first
                if (inCondition) {
                    finalizeCondition(/*forceNegate*/false);
                }
                logicalOp = token; // e.g. AND or OR
            }
            continue;
        }

        // 2) If not a logical operator, we are parsing a condition piece
        if (!inCondition) {
            // Start a new condition
            inCondition   = true;
            currentColumn = token;
        }
        else if (currentOp.empty()) {
            // Check for multi-character operators
            // e.g. if token == ">" and next token == "=", combine into ">="
            if ((token == ">" || token == "<" || token == "!") &&
                i + 1 < tokens.size() && tokens[i+1] == "=") {
                currentOp = token + "=";
                ++i; // Skip the next token
            }
            else if (token == "IN") {
                // Check if we had a preceding NOT
                currentOp = "IN";
            }
            else {
                // Simple operator: =, >, <, >=, <=, !=
                currentOp = token;
            }
        }
        else {
            // We have column & operator, now parse value(s)
            if (currentOp == "IN") {
                // We might see something like: ( 'Anton', 'Maria');
                // or multiple tokens with parentheses

                // Remove leading '(' if present
                if (!token.empty() && token.front() == '(') {
                    token.erase(token.begin());
                }
                // Remove trailing ')' or ';' or ',' if present
                while (!token.empty() &&
                       (token.back() == ')' || token.back() == ';' || token.back() == ',')) {
                    token.pop_back();
                }

                // Now we might have a chunk like "'Anton'," or "'Maria'"
                // Split by comma if multiple items remain in the same token
                std::vector<std::string> pieces = split(token, ',');
                for (auto &p : pieces) {
                    p = trim(p);       // remove extra spaces
                    p = stripQuotes(p);
                    if (!p.empty()) {
                        inValueList.push_back(p);
                    }
                }

                // If the original token had a `)`, we assume IN list is done
                if (tokens[i].find(')') != std::string::npos) {
                    finalizeCondition(/*forceNegate*/nextConditionShouldNegate);
                }
            }
            else {
                // Single value for =, !=, >, <, etc.
                // We want to handle quoted strings, possibly spanning multiple tokens
                // But because we do naive space-splitting, let's do a simpler approach:
                if (inQuote || (token.front() == '\'' || token.front() == '"')) {
                    // Append to valueBuffer
                    if (!valueBuffer.empty()) valueBuffer += " ";
                    valueBuffer += token;

                    // If we have matching quotes at start and end, finalize
                    char start = valueBuffer.front();
                    char end   = valueBuffer.back();
                    if ((start == '\'' && end == '\'') || (start == '"' && end == '"')) {
                        // We have a fully quoted string
                        currentValue = stripQuotes(valueBuffer);
                        finalizeCondition(/*forceNegate*/nextConditionShouldNegate);
                    }
                    else {
                        // We haven't closed the quote yet
                        inQuote = true;
                    }
                }
                else {
                    // Unquoted single token
                    currentValue = token;
                    // Condition ends here
                    finalizeCondition(/*forceNegate*/nextConditionShouldNegate);
                }
            }
        }
    }

    // If we still have a condition open (e.g. last token was an operator or partial value)
    if (inCondition && !currentOp.empty()) {
        // Possibly we were reading a quoted string that ended exactly at the last token
        if (!valueBuffer.empty()) {
            currentValue = stripQuotes(valueBuffer);
        }
        finalizeCondition(/*forceNegate*/nextConditionShouldNegate);
    }

    return conditions;
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
    const Column& column = *colIt;
    const Value& value = row.values[colIndex];

    bool result = false;
    try {
        // Handle the "IN" case separately
        if (cond.op == "IN") {
            if (std::holds_alternative<std::string>(value)) {
                const std::string& strValue = std::get<std::string>(value);
                result = (std::find(cond.inValues.begin(), cond.inValues.end(), strValue)
                          != cond.inValues.end());
            } else if (std::holds_alternative<int>(value)) {
                int intValue = std::get<int>(value);
                for (const auto &iv : cond.inValues) {
                    try {
                        if (intValue == std::stoi(iv)) {
                            result = true;
                            break;
                        }
                    } catch (...) {
                        // skip non-convertible
                    }
                }
            } else if (std::holds_alternative<float>(value)) {
                float floatValue = std::get<float>(value);
                for (const auto &iv : cond.inValues) {
                    try {
                        float cmp = std::stof(iv);
                        if (std::fabs(floatValue - cmp) < 1e-6) {
                            result = true;
                            break;
                        }
                    } catch (...) {
                        // skip
                    }
                }
            } else if (std::holds_alternative<char>(value)) {
                char charValue = std::get<char>(value);
                for (const auto &iv : cond.inValues) {
                    if (!iv.empty() && iv[0] == charValue) {
                        result = true;
                        break;
                    }
                }
            }
        } else {
            // Handle standard operators: =, !=, >, <, >=, <=
            if (column.type == DataType::DATE) {
                // Date comparison (lexicographical)
                if (!std::holds_alternative<std::string>(value)) {
                    throw std::runtime_error("Invalid value for DATE column: " + cond.column);
                }
                const std::string& dateValue = std::get<std::string>(value);

                if      (cond.op == "=")  result = (dateValue == cond.value);
                else if (cond.op == "!=") result = (dateValue != cond.value);
                else if (cond.op == ">")  result = (dateValue >  cond.value);
                else if (cond.op == "<")  result = (dateValue <  cond.value);
                else if (cond.op == ">=") result = (dateValue >= cond.value);
                else if (cond.op == "<=") result = (dateValue <= cond.value);
                else {
                    throw std::runtime_error("Unsupported operator '" + cond.op + "' for DATE type.");
                }
            }
            else if (std::holds_alternative<int>(value)) {
                int intValue = std::get<int>(value);
                int condValue = std::stoi(cond.value);

                if      (cond.op == "=")  result = (intValue == condValue);
                else if (cond.op == "!=") result = (intValue != condValue);
                else if (cond.op == ">")  result = (intValue >  condValue);
                else if (cond.op == "<")  result = (intValue <  condValue);
                else if (cond.op == ">=") result = (intValue >= condValue);
                else if (cond.op == "<=") result = (intValue <= condValue);
            }
            else if (std::holds_alternative<float>(value)) {
                float floatValue = std::get<float>(value);
                float condValue = std::stof(cond.value);

                if      (cond.op == "=")  result = (std::fabs(floatValue - condValue) < 1e-6);
                else if (cond.op == "!=") result = (std::fabs(floatValue - condValue) >= 1e-6);
                else if (cond.op == ">")  result = (floatValue >  condValue);
                else if (cond.op == "<")  result = (floatValue <  condValue);
                else if (cond.op == ">=") result = (floatValue >= condValue);
                else if (cond.op == "<=") result = (floatValue <= condValue);
            }
            else if (std::holds_alternative<char>(value)) {
                char charValue = std::get<char>(value);
                if (cond.value.empty()) {
                    throw std::runtime_error("Empty string in WHERE clause for char comparison.");
                }

                if      (cond.op == "=")  result = (charValue == cond.value[0]);
                else if (cond.op == "!=") result = (charValue != cond.value[0]);
            }
            else if (std::holds_alternative<std::string>(value)) {
                const std::string& strValue = std::get<std::string>(value);

                if      (cond.op == "=")  result = (strValue == cond.value);
                else if (cond.op == "!=") result = (strValue != cond.value);
            }
        }
    }
    catch (...) {
        throw std::runtime_error("Type mismatch in WHERE clause: Cannot compare '"
                                 + cond.value + "' to column '" + cond.column + "'");
    }

    // Apply negation if Condition says so (for "NOT IN" or "NOT <op>")
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
                // For the first condition or if no logicalOp is given
                overallResult = condResult;
            }
        }

        if (overallResult) {
            filteredRows.push_back(row);
        }
    }

    return filteredRows;
}
