#include "json.h"

using namespace std;

namespace Json
{

Node LoadArray(istream &input)
{
    vector<Node> result;

    for (char c; input >> c && c != ']';)
    {
        if (c != ',')
        {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(std::move(result));
}

Node LoadBool(istream &input)
{
    std::string s;
    while (isalpha(input.peek()))
    {
        s.push_back(input.get());
    }
    return Node(s == "true");
}

Node LoadNumber(istream &input)
{
    double result = 0.0;
    input >> result;
    return Node(result);
}

Node LoadString(istream &input)
{
    std::string line;
    getline(input, line, '"');
    return Node(std::move(line));
}

Node LoadDict(istream &input)
{
    Dict result;

    for (char c; input >> c && c != '}';)
    {
        if (c == ',')
        {
            input >> c;
        }

        std::string key = LoadString(input).AsString();
        input >> c;
        result.emplace(std::move(key), LoadNode(input));
    }

    return Node(std::move(result));
}

Node LoadNode(istream &input)
{
    char c;
    input >> c;

    if (c == '[')
    {
        return LoadArray(input);
    }
    else if (c == '{')
    {
        return LoadDict(input);
    }
    else if (c == '"')
    {
        return LoadString(input);
    }
    else if (c == 't' || c == 'f')
    {
        input.putback(c);
        return LoadBool(input);
    }
    else
    {
        input.putback(c);
        return LoadNumber(input);
    }
}

Document Load(istream &input)
{
    return Document{LoadNode(input)};
}

struct PrintContext
{
    std::ostream& out;
    int indent = 0;
    int indent_step = 4;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indent + indent_step, indent_step};
    }
};

void PrintNode(const Json::Node &node, const PrintContext& ctx);

template <typename Value>
void PrintValue(const Value &value, const PrintContext& ctx) {
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
        case '\\':
            out.put('\\');
        default:
            out.put(c);
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
    for (const Node& node : nodes)
    {
        if (first)
        {
            first = false;
        } else {
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
void PrintValue<Dict>(const Dict& dict, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : dict)
    {
        if (first)
        {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintString(key, out);
        out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit([&ctx](const auto& value) { 
                PrintValue(value, ctx);
            },
            node.GetBase());
}

void Print(const Document& document, std::ostream& output) {
    PrintNode(document.GetRoot(), PrintContext{output});
}

}
