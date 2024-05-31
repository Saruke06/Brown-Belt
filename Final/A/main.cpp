#include <iostream>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>

using namespace std;

template <typename Number>
Number ReadNumberOnLine(istream& is) {
    Number result;
    is >> result;
    string dummy;
    getline(is, dummy);
    return result;
}

pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimeter = " ") {
    const size_t pos = s.find(delimeter);
    if (pos == s.npos) {
        return {s, nullopt};
    } else {
        return {s.substr(0, pos), s.substr(pos + delimeter.length())};
    }
}

pair<string_view, string_view> SplitTwo(string_view s, string_view delimeter = " ") {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimeter);
    return {lhs, rhs_opt.value_or("")};
}

string_view ReadToken(string_view& s, string_view delimeter = " ") {
    const auto [lhs, rhs] = SplitTwo(s, delimeter);
    s = rhs;
    return lhs;
}

struct Request;
using RequestHolder = unique_ptr<Request>;

struct Request {
    enum class Type {
        STOP,
        BUS
    };

    Request(Type type) : type(type) {}
    static RequestHolder Create(Request::Type type);
    virtual void ParseFrom(string_view input) = 0;
    virtual ~Request() = default;

    const Type type;
};

struct ModifyRequest : Request {
    using Request::Request;
    virtual void Process(const TransportDatabase& db) const = 0;
};

struct AddBusRequest : ModifyRequest {
    AddBusRequest() : ModifyRequest(Type::BUS) {}

    void ParseFrom(string_view input) override {
        // BUS 256
    }

    void Process(TransportDatabase& db) const override {
        // db.AddBus();
    }
};

struct Stop {
    string name;
};

struct Bus {
    string name;
};


class TransportDatabase {
public:
    TransportDatabase() = default;

    void BuildBase(istream& input = cin) {
        const size_t updates_count = ReadNumberOnLine<size_t>(input);
        for (size_t i = 0; i < updates_count; ++i) {

        }
    }

private:
unordered_map<string, Stop> stops;
unordered_map<string, Bus> buses;

};

void PrintResponses(const vector<string>& responses, ostream& output = cout) {
    for (const string& response : responses) {
        output << response << '\n';
    }
}

int main() {
    TransportDatabase db;

    db.BuildBase();
    
    const auto requests = ReadRequests();
    const auto responses = ProcessRequests(db, requests);
    PrintResponses(responses);

    return 0;
}