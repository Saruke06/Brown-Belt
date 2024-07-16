#include "json.h"

using namespace std;

namespace Json {

  Document::Document(Node root) : root(move(root)) {
  }

  const Node& Document::GetRoot() const {
    return root;
  }

  Node LoadNode(istream& input);

  Node LoadArray(istream& input) {
    vector<Node> result;

    for (char c; input >> c && c != ']'; ) {
      if (c != ',') {
        input.putback(c);
      }
      result.push_back(LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
      result *= 10;
      result += input.get() - '0';
    }
    return Node(result);
  }

  Node LoadDouble(istream& input) {
    double result = 0;
    input >> result;
    return Node(result);
  }

  Node LoadNumber(istream& input) {
    string number_str;
    bool is_double = false;
    char c;
    
    while (input >> c && (isdigit(c) || c == '.' || c == '+' || c == '-')) {
      if (c == '.') {
        is_double = true;
      }
      number_str.push_back(c);
    }

    istringstream is(number_str);

    if (is_double) {
      return LoadDouble(is);
    } else {
      return LoadInt(is);
    }
  }

  Node LoadBool(istream& input) {
    string bool_str;
    char c;

    while (input >> c && isalpha(c)) {
      bool_str.push_back(c);
    }

    if (bool_str == "true") {
      return Node(true);
    } else {
      return Node(false);
    }
  }

  Node LoadValue(istream& input) {
    char c;

    if (isdigit(input.peek()) || input.peek() == '-') {
      return LoadNumber(input);
    } else return LoadBool(input);
  }

  Node LoadString(istream& input) {
    string line;
    getline(input, line, '"');
    return Node(move(line));
  }

  Node LoadDict(istream& input) {
    map<string, Node> result;

    for (char c; input >> c && c != '}'; ) {
      if (c == ',') {
        input >> c;
      }

      string key = LoadString(input).AsString();
      input >> c;
      result.emplace(move(key), LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
      return LoadArray(input);
    } else if (c == '{') {
      return LoadDict(input);
    } else if (c == '"') {
      return LoadString(input);
    } else {
      input.putback(c);
      return LoadValue(input);
    }
  }

  Document Load(istream& input) {
    return Document{LoadNode(input)};
  }


  struct VariantPrinter {
    std::ostream& out;

    VariantPrinter(std::ostream& out) : out(out) {}

    void operator() (int value) const {
        out << value;
    }

    void operator() (double value) const {
        out << value;
    }

    void operator() (bool value) const {
        out << std::boolalpha << value;
    }

    void operator() (const std::string& value) const {
        out << value;
    }

    void operator() (const std::vector<Node>& nodes) const {
      out << "[";
      for (size_t i = 0; i < nodes.size(); ++i) {
          out << nodes[i];
          if (i != nodes.size() - 1) {
              out << ", ";
          }
      }
      out << "]";
    }

    void operator() (const std::map<std::string, Node>& nodes) const {
      out << "{";
      for (const auto& [key, value] : nodes) {
          out << key << ": " << value;
      }
      out << "}";
    }
  };

  template <typename... Ts>
  std::ostream& operator<<(std::ostream& output, const std::variant<Ts...>& variant) {
      std::visit(VariantPrinter(output), variant);
      return output;
  }

  ostream& operator<<(ostream& output, const Node& node) {
      return output << node;
  }

}
