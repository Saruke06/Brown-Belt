#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <sstream>

namespace Json {

  class Node;
  using Dict = std::map<std::string, Node>;

  class Node : std::variant<std::vector<Node>, Dict, bool, int, double, std::string> {
  public:
    using variant::variant;
    const variant& GetBase() const { return *this; }

    const auto& AsArray() const { return std::get<std::vector<Node>>(*this); }
    const auto& AsMap() const { return std::get<Dict>(*this); }
    bool AsBool() const { return std::get<bool>(*this); }
    int AsInt() const { return std::get<int>(*this); }
    double AsDouble() const { 
        return std::holds_alternative<double>(*this) ? std::get<double>(*this) : std::get<int>(*this);
    }
    const auto& AsString() const { return std::get<std::string>(*this); }
  };

  class Document {
  public:
    explicit Document(Node root) : root(std::move(root)) {}

    const Node& GetRoot() const {
      return root;
    }

  private:
    Node root;
  };

  Document Load(std::istream& input = std::cin);

}
