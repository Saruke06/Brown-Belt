#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <optional>
#include <vector>


template <typename Number>
Number ReadNumberOnLine(std::istream& is) {
    Number result;
    is >> result;
    std::string dummy;
    getline(is, dummy);
    return result;
}

std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(std::string_view s, std::string_view delimeter = " ") {
    const size_t pos = s.find(delimeter);
    if (pos == s.npos) {
        return {s, std::nullopt};
    } else {
        return {s.substr(0, pos), s.substr(pos + delimeter.length())};
    }
}

std::pair<std::string_view, std::string_view> SplitTwo(std::string_view s, std::string_view delimeter = " ") {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimeter);
    return {lhs, rhs_opt.value_or("")};
}

std::string_view ReadToken(std::string_view& s, std::string_view delimeter = " ") {
    const auto [lhs, rhs] = SplitTwo(s, delimeter);
    s = rhs;
    return lhs;
}