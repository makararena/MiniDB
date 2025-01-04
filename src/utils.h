#pragma once
#include <string>
#include <vector>

// String utilities
std::string trim(const std::string& str);
std::string toUpperCase(const std::string& str);
std::string toLowerCase(const std::string& str);
std::string removeTrailingSemicolon(const std::string& str);

// Tokenization and splitting utilities
std::vector<std::string> split(const std::string& s, char delimiter);

// Quoting utilities
bool isQuoted(const std::string& s); // Renamed to match implementation
std::string stripQuotes(const std::string& s);

// Logical operator utility
bool isLogical(const std::string& token);

// Case-insensitive comparison utility
bool caseInsensitiveEquals(const std::string& a, const std::string& b);

// Manual character uppercasing (used internally)
char toUpperManual(char c);

// Keyword normalization utility
std::string normalizeKeywords(const std::string& input, const std::vector<std::string>& keywords);