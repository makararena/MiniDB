#pragma once
#include <string>
#include <vector>

enum class CaseType { UPPER, LOWER };

// Simply trims the string from the left and right deleting all spaces and "\t" and "\n" and "\r"
std::string trim(const std::string& str);

// Case conversion utility (https://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case)
std::string toCase(const std::string& str, CaseType caseType);

// Deletes trailing ";"
std::string removeTrailingSemicolon(const std::string& str);

// Splits the string by delimiter returning vector of strings
std::vector<std::string> split(const std::string& s, char delimiter);

// A small helper to see if a string starts and ends with matching quotes
bool isQuoted(const std::string& s);

// A helper to remove surrounding quotes if present
std::string stripQuotes(const std::string& s);

// A helper to see if token is a known logical operator
bool isLogical(const std::string& token);

// Case-insensitive equality check (same letters, ignoring case).
bool caseInsensitiveEquals(const std::string& a, const std::string& b);

// Converts chat from smaller case to upper case if the char in small case (a -> A, A -> A)
char toUpperManual(char c);

// normalizes input command from input = "select name from users order by age";
// to "SELECT name FROM users ORDER BY age"
std::string normalizeKeywords(const std::string& input);

// Runs basic tests to check whether application is still working
void runTests();

// Display utilities - first one displays text header of the application and the second shows help text
void displayHeader();
void displayHelp();

// https://www.quora.com/Why-do-we-use-h-files-in-C-only-for-declarations-of-a-function-Why-are-they-not-defined-in-the-h-file-as-well
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