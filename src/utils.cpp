#include <iostream>
#include <sstream>
#include <vector>

auto trim(const std::string& str) -> std::string{
    // std::size_t is an unsigned integer type specifically designed for representing sizes and indexes in arrays, strings, and other containers.
    // and also std::size_t can resperesnt much larger size than int

    std::size_t start = str.find_first_not_of(" \t\n\r"); // find first non-whitespace character ( ' ' (space); '\t' (tab); '\n' (newline); '\r' (carriage return))
    // If the string contains only whitespace, find_first_not_of will return std::string::npos.
    if (start == std::string::npos) return ""; // https://www.naukri.com/code360/library/string-npos-in-cpp (link to the what is npos) (represents the maximum constant for the string in c++)

    std::size_t end = str.find_last_not_of(" \t\n\r"); // scans the string from right to left looking for the last character than is not in the set of " \t\n\r"
    return str.substr(start, end - start + 1); // return the trimmed string
}

// сonvert the string to the upper case
std::string toUpperCase(const std::string& str) {
    std::string upperStr = str;
    std::transform(
        upperStr.begin(),
        upperStr.end(),
        upperStr.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return upperStr;
}

// сonvert the string to the lower case
std::string toLowerCase(const std::string& str) {
    std::string upperStr = str;
    std::transform(
        upperStr.begin(),
        upperStr.end(),
        upperStr.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return upperStr;
}

// split the string by the dilimeter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens; //  A vector to store the resulting substrings
    std::string token; // A temporary string used to hold each substring as it is extracted
    std::stringstream ss(s); //  allows the string to be treated as a stream, enabling easy parsing

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

// Removes the trailing semicolon from a string, if present
std::string removeTrailingSemicolon(const std::string& str) {
    if (!str.empty() && str.back() == ';') {
        return str.substr(0, str.size() - 1); // Return the string without the last character
    }
    return str; // Return the original string if no semicolon is found
}

std::string normalizeKeywords(const std::string& input, const std::vector<std::string>& keywords) {
    // Split the input string into words
    std::vector<std::string> words;
    std::stringstream ss(input);
    std::string word;
    while (ss >> word) {
        words.push_back(word);
    }

    // Process each word
    for (auto& w : words) {
        for (const auto& keyword : keywords) {
            if (toUpperCase(w) == toUpperCase(keyword)) {
                w = toUpperCase(keyword); // Normalize the keyword
                break;
            }
        }
    }

    // Reconstruct the string from the vector of words
    std::ostringstream normalized;
    for (size_t i = 0; i < words.size(); ++i) {
        normalized << words[i];
        if (i < words.size() - 1) {
            normalized << " ";
        }
    }

    return normalized.str();
}

