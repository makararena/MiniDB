#pragma once
#include <string>
#include <vector>
#include <fmt/format.h>
#include <variant>

std::string trim(const std::string& str);
std::string toUpperCase(const std::string& str);
std::string toLowerCase(const std::string& str);
std::vector<std::string> split(const std::string& s, char delimiter);
std::string removeTrailingSemicolon(const std::string& str);
std::string normalizeKeywords(const std::string& input, const std::vector<std::string>& keywords);

// Custom formatter specialization for std::variant<int, float, char, std::string>
template <>
struct fmt::formatter<std::variant<int, float, char, std::string>> {

    // The parse function processes any custom format specifiers (e.g., "{:x}" or "{:.2f}").
    // In this implementation, we simply ignore format specifiers and use the default "{}" format.
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin(); // Return the beginning of the context to indicate no special parsing is required.
    }

    // The format function defines how to format the std::variant when used with fmt::format.
    // It is called when the variant needs to be formatted as a string.
    template <typename FormatContext>
    auto format(const std::variant<int, float, char, std::string>& var, FormatContext& ctx) const -> decltype(ctx.out()) {
        // Use std::visit to handle the active type in the variant.
        return std::visit(
            [&](const auto& value) {
                // For the active type, call fmt::format_to to format the value as a string.
                // The "{}" format specifier is used, which applies default formatting for the type.
                return fmt::format_to(ctx.out(), "{}", value);
            },
            var // The std::variant being formatted.
        );
    }
};

