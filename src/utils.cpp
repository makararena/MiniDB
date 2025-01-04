#include <iostream>
#include <sstream>
#include <vector>

// Just simple trim-helper function
std::string trim(const std::string& str) {
    int start = 0;
    int end = str.length() - 1;

    while (start <= end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r')) {
        start++;
    }

    while (end >= start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\n' || str[end] == '\r')) {
        end--;
    }

    return str.substr(start, end - start + 1);
}


// сonvert the string to the upper case (https://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case)
std::string toUpperCase(const std::string& str) {
    std::string upperStr = str;
    std::transform(
        upperStr.begin(),
        upperStr.end(),
        upperStr.begin(),
        [](unsigned char c) { return std::toupper(c); });

    return upperStr;
}

// сonvert the string to the lower case (https://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case)
std::string toLowerCase(const std::string& str) {
    std::string lowerStr = str;
    std::transform(
        lowerStr.begin(),
        lowerStr.end(),
        lowerStr.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return lowerStr;
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

// A small helper to see if a string starts and ends with matching quotes
bool isQuoted(const std::string& s) {
    if (s.size() < 2) return false;
    return ((s.front() == '\'' && s.back() == '\'') ||
            (s.front() == '"'  && s.back() == '"'));
}

// A helper to remove surrounding quotes if present
std::string stripQuotes(const std::string& s) {
    if (isQuoted(s)) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

// A helper to see if token is a known logical operator
bool isLogical(const std::string& token) {
    return (token == "AND" || token == "OR" || token == "NOT");
}

char toUpperManual(char c) {
    // 'a'..'z' -> 'A'..'Z'
    if (c >= 'a' && c <= 'z') {
        c = static_cast<char>(c - 'a' + 'A');
    }
    return c;
}

// Case-insensitive equality check (same letters, ignoring case).
bool caseInsensitiveEquals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;

    for (size_t i = 0; i < a.size(); ++i) {
        char ca = a[i];
        char cb = b[i];
        ca = toUpperManual(ca);
        cb = toUpperManual(cb);
        if (ca != cb) return false;
    }
    return true;
}

std::string normalizeKeywords(const std::string& input,
                              const std::vector<std::string>& rawKeywords)
{
    // Separate single-word from multi-word keywords
    std::vector<std::string> singleWordKws;
    std::vector<std::string> multiWordKws;

    for (size_t i = 0; i < rawKeywords.size(); ++i) {
        // If a keyword has a space, we treat it as multi-word
        if (rawKeywords[i].find(' ') != std::string::npos) {
            multiWordKws.push_back(rawKeywords[i]);
        } else {
            singleWordKws.push_back(rawKeywords[i]);
        }
    }

    // Split the input into tokens (by spaces)
    std::vector<std::string> tokens = split(input, ' ');

    // We'll build a new list of tokens with the updated cases
    std::vector<std::string> resultTokens;
    resultTokens.reserve(tokens.size());

    for (size_t i = 0; i < tokens.size(); ++i) {
        bool foundMulti = false;

        // 1) Try matching each multi-word keyword at position i
        for (size_t mk = 0; mk < multiWordKws.size(); ++mk) {
            std::vector<std::string> parts = split(multiWordKws[mk], ' ');
            // Check if we have enough tokens left for this multi-word
            if (i + parts.size() <= tokens.size()) {
                bool match = true;
                for (size_t p = 0; p < parts.size(); ++p) {
                    if (!caseInsensitiveEquals(tokens[i + p], parts[p])) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    // We found a multi-word match. Add them in uppercase
                    // or exactly as in rawKeywords, e.g. "ORDER BY"
                    resultTokens.push_back(multiWordKws[mk]);
                    // Skip ahead past these tokens
                    i += (parts.size() - 1);
                    foundMulti = true;
                    break;
                }
            }
        }
        if (foundMulti) {
            continue; // move to next token after multi-word match
        }

        // 2) If no multi-word match, check for single-word
        bool foundSingle = false;
        for (size_t sk = 0; sk < singleWordKws.size(); ++sk) {
            if (caseInsensitiveEquals(tokens[i], singleWordKws[sk])) {
                // Use the exact form from singleWordKws, which is uppercase
                resultTokens.push_back(singleWordKws[sk]);
                foundSingle = true;
                break;
            }
        }

        if (!foundSingle) {
            // No match at all, push original token
            resultTokens.push_back(tokens[i]);
        }
    }

    // Rebuild the string with spaces
    std::stringstream out;
    for (size_t i = 0; i < resultTokens.size(); ++i) {
        if (i > 0) out << " ";
        out << resultTokens[i];
    }
    return out.str();
}