#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
class Node : std::variant<std::vector<Node>, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    [[nodiscard]] auto GetBase() const -> const variant& { return *this; }

    [[nodiscard]] auto AsArray() const -> const auto& { return std::get<std::vector<Node>>(*this); }
    [[nodiscard]] auto AsMap() const -> const auto& { return std::get<Dict>(*this); }
    [[nodiscard]] auto AsBool() const -> bool { return std::get<bool>(*this); }
    [[nodiscard]] auto AsInt() const -> int { return static_cast<int>(std::get<double>(*this)); }
    [[nodiscard]] auto AsDouble() const -> double
    {
        return std::holds_alternative<double>(*this) ? std::get<double>(*this) : std::get<int>(*this);
    }
    [[nodiscard]] auto AsString() const -> const auto& { return std::get<std::string>(*this); }
    const variant& GetValue() const {
        return *this;
    }
};

class Document {
public:
    explicit Document(Node root)
        : root(move(root))
    {
    }

    [[nodiscard]] auto GetRoot() const -> const Node&
    {
        return root;
    }

private:
    Node root;
};

auto LoadNode(std::istream& input) -> Node;

auto Load(std::istream& input) -> Document;


void Print(const Document& document, std::ostream& output);

}
