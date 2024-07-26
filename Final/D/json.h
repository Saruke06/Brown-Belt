#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <sstream>

namespace Json {

  class Node : std::variant<std::vector<Node>,
                            std::map<std::string, Node>,
                            int,
                            std::string,
                            double,
                            bool> {
  public:
    using variant::variant;

    const auto& AsArray() const {
      return std::get<std::vector<Node>>(*this);
    }
    const auto& AsMap() const {
      return std::get<std::map<std::string, Node>>(*this);
    }
    int AsInt() const {
      return std::get<int>(*this);
    }
    const auto& AsString() const {
      return std::get<std::string>(*this);
    }
    double AsDouble() const {
      return std::get<double>(*this);
    }
    bool AsBool() const {
      return std::get<bool>(*this);
    }
  };

  class Document {
  public:
    explicit Document(Node root);

    const Node& GetRoot() const;

  private:
    Node root;
  };

  Document Load(std::istream& input = std::cin);

  std::ostream& operator<<(std::ostream& output, const Node& node);
}

// // Специализация std::variant_size и std::variant_alternative для Json::Node
// namespace std {

//   template <>
//   struct variant_size<Json::Node> : variant_size<typename Json::Node::variant> {};

//   template <std::size_t I>
//   struct variant_alternative<I, Json::Node> : variant_alternative<I, typename Json::Node::variant> {};

//   template <>
//   inline constexpr std::size_t variant_size_v<Json::Node> = variant_size<Json::Node>::value;
// }