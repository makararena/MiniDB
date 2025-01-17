#pragma once
#include <string>
#include <vector>
#include <stdexcept>

enum class CaseType { UPPER, LOWER };

// Trims the string from the left and right, removing spaces, "\t", "\n", and "\r".
std::string trim(const std::string& str);

// Converts the case of the string to either uppercase or lowercase.
// (Reference: https://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case)
std::string toCase(const std::string& str, CaseType caseType);

// Removes a trailing semicolon from the string, if present.
std::string removeTrailingSemicolon(const std::string& str);

// Splits the string by a specified delimiter and returns a vector of substrings.
std::vector<std::string> split(const std::string& s, char delimiter);

// Checks if a string starts and ends with matching quotes.
bool isQuoted(const std::string& s);

// Removes surrounding quotes from a string, if they exist.
std::string stripQuotes(const std::string& s);

// Checks if a token is a known logical operator (e.g., AND, OR, NOT).
bool isLogical(const std::string& token);

// Compares two strings for equality, ignoring case differences.
bool caseInsensitiveEquals(const std::string& a, const std::string& b);

// Converts a character from lowercase to uppercase, if applicable.
char toUpperManual(char c);

// Normalizes input SQL-like commands by converting keywords to uppercase.
// For example: "select name from users order by age" becomes "SELECT name FROM users ORDER BY age".
std::string normalizeKeywords(const std::string& input);

// Runs basic tests to verify the application's functionality.
void runTests();

// Display utilities:
// - Displays the application header.
// - Displays help text with a list of available commands.
void displayHeader();
void displayHelp();

// A generic template function to compare two values using a specified operator (e.g., =, !=, >, <, >=, <=).
// Throws an exception if the operator is unsupported.
template <typename T>
bool compareValues(const T& value, const T& condValue, const std::string& op) {
    if      (op == "=")  return value == condValue;
    else if (op == "!=") return value != condValue;
    else if (op == ">")  return value > condValue;
    else if (op == "<")  return value < condValue;
    else if (op == ">=") return value >= condValue;
    else if (op == "<=") return value <= condValue;
    throw std::runtime_error("Unsupported operator '" + op + "'");
}
