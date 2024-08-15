#include "json.h"

using namespace std;

namespace Json {

auto LoadArray(istream& input) -> Node
{
    vector<Node> result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(move(result));
}

auto LoadBool(istream& input) -> Node
{
    string s;
    while (isalpha(input.peek()) != 0) {
        s.push_back(input.get());
    }
    return Node(s == "true");
}

auto LoadNumber(istream& input) -> Node
{
    double result = 0.0;
    input >> result;
    return Node(result);
}

auto LoadString(istream& input) -> Node
{
    string line;
    getline(input, line, '"');
    return Node(move(line));
}

auto LoadDict(istream& input) -> Node
{
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.emplace(move(key), LoadNode(input));
    }

    return Node(move(result));
}

auto LoadNode(istream& input) -> Node
{
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    }
    if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

auto Load(istream& input) -> Document
{
    return Document { LoadNode(input) };
}


struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }
};

void PrintNode(const Node& value, const PrintContext& ctx);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintString(const std::string& value, std::ostream& out) {
    out.put('"');
    for (const char c : value) {
        switch (c) {
        case '\r':
            out << "\\r"sv;
            break;
        case '\n':
            out << "\\n"sv;
            break;
        case '"':
            // Символы " и \ выводятся как \" или \\, соответственно
            [[fallthrough]];
        case '\\':
            out.put('\\');
            [[fallthrough]];
        default:
            out.put(c);
            break;
        }
    }
    out.put('"');
}

template <>
void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
    PrintString(value, ctx.out);
}

template <>
void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

// В специализаци шаблона PrintValue для типа bool параметр value передаётся
// по константной ссылке, как и в основном шаблоне.
// В качестве альтернативы можно использовать перегрузку:
// void PrintValue(bool value, const PrintContext& ctx);
template <>
void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
    ctx.out << (value ? "true"sv : "false"sv);
}

template <>
void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        }
        else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}

template <>
void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        }
        else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintString(key, ctx.out);
        out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value) {
            PrintValue(value, ctx);
        },
        node.GetValue());
}
void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{ output });
}

}
