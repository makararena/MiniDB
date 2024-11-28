#pragma once
#include <string>
#include <vector>
#include <fmt/format.h>
#include <variant>

std::string trim(const std::string& str);
std::string toUpperCase(const std::string& str);
std::vector<std::string> split(const std::string& s, char delimiter);
std::string removeTrailingSemicolon(const std::string& str);

// Custom formatter specialization for std::variant
template <>
struct fmt::formatter<std::variant<int, float, char, std::string>> {
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::variant<int, float, char, std::string>& var, FormatContext& ctx) const -> decltype(ctx.out()) {
        return std::visit([&](const auto& value) {
            return fmt::format_to(ctx.out(), "{}", value);
        }, var);
    }
};
