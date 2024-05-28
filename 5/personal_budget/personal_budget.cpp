#include <iostream>
#include <ctime>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

using namespace std;

string_view ReadToken(string_view& str, string_view delimeter) {
    
}

class Date {
public:
    static Date ParseFrom(string_view str) {
        const int year = ConvertToInt(ReadToken(str, "-"));
    }

    time_t AsTimestamp() const {
        std::tm t;
        t.tm_sec   = 0;
        t.tm_min   = 0;
        t.tm_hour  = 0;
        t.tm_mday  = day_;
        t.tm_mon   = month_ - 1;
        t.tm_year  = year_ - 1900;
        t.tm_isdst = 0;
        return mktime(&t);
    }
private:
    int day_;
    int month_;
    int year_;
};

int ComputeDaysDiff(const Date& date_to, const Date& date_from) {
    const time_t timestamp_to = date_to.AsTimestamp();
    const time_t timestamp_from = date_from.AsTimestamp();
    static const int SECONDS_IN_DAY = 60 * 60 * 24;
    return (timestamp_to - timestamp_from) / SECONDS_IN_DAY;
}

struct Request;
using RequestHolder = unique_ptr<Request>;

struct Request {
    enum class Type {
        COMPUTE_INCOME,
        EARN,
        PAY_TAX
    };

    Request(Type type) : type(type) {}
    static RequestHolder Create(Type type);
    virtual void ParseFrom(string_view input) = 0;
    virtual ~Request() = default;

    const Type type;
};

const unordered_map<string_view, Request::Type> STR_TO_REQUEST_TYPE = {
    {"ComputeIncome", Request::Type::COMPUTE_INCOME},
    {"Earn", Request::Type::EARN},
    {"PayTax", Request::Type::PAY_TAX}
};

template <typename ResultType>
struct ReadRequest : Request {
    using Request::Request;
    virtual ResultType Process(const BudgetManager& manager) const = 0; 
};

struct ModifyRequest : Request {
    using Request::Request;
    virtual void Process(BudgetManager& manager) const = 0;
};

struct ComputeIncomeRequest : ReadRequest<double> {
    ComputeIncomeRequest() : ReadRequest(Type::COMPUTE_INCOME) {}

    void ParseFrom(string_view input) override {
        date_from = ReadT
    }

    Date date_from;
    Date date_to;
};


Request ReadRequest(istream& is = cin) {
    Request query;
    string type;
    is >> type;
    if (type == "ComputeIncome") {
        query.type = RequestType::COMPUTE_INCOME;
        is >> query.from >> query.to;
    } else if (type == "Earn") {
        query.type = RequestType::EARN;
        is >> query.from >> query.to >> query.value;
    } else if (type == "PayTax") {
        query.type = RequestType::PAY_TAX;
        is >> query.from >> query.to >> query.value;
    } else {
        return {s.substr(0, pos), s.substr(pos + delimiter.length())};
    }
}

pair<string_view, string_view> SplitTwo(string_view s, string_view delimiter = " ") {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    return {lhs, rhs_opt.value_or("")};
}

string_view ReadToken(string_view& s, string_view delimiter = " ") {
    const auto [lhs, rhs] = SplitTwo(s, delimiter);
    s = rhs;
    return lhs;
}

template <typename Number>
Number ReadNumberOnLine(istream& is = cin) {
    Number number;
    is >> number;
    string dummy;
    getline(is, dummy);
    return number;
}

const unordered_map<string_view, Request::Type> STR_TO_REQUEST_TYPE = {
    {"ComputeIncome", RequestType::COMPUTE_INCOME},
    {"Earn", RequestType::EARN},
    {"PayTax", RequestType::PAY_TAX}
};


optional<Request::Type>

Request ParseRequest(string_view request_str) {
    const auto request_type = ConvertRequestTypeFromString(ReadToken(request_str));

}

vector<Request> ReadRequests(istream& is = cin) {
    const size_t request_count = ReadNumberOnLine<size_t>(is);

    vector<Request> requests;
    requests.reserve(request_count);

    for (int i = 0; i < request_count; ++i) {
        string request_str;
        getline(is, request_str);
        if (auto request = ParseRequest(request_str)) {
            requests.push_back(move(request));
        }
    }
    return requests;
}

vector<double> ProcessRequests(const vector<Request>& requests) {
    vector<double> responses;
    for (const auto& request : requests) {
        switch (request.type) {
            case RequestType::COMPUTE_INCOME:
                responses.push_back(ComputeIncome(request.from, request.to));
                break;
            case RequestType::EARN:
                Earn(request.from, request.to, request.value);
                break;
            case RequestType::PAY_TAX:
                PayTax(request.from, request.to, request.value);
                break;
        }
    }
    return responses;
}

void PrintResponses(const vector<double>& responses, ostream& os = cout) {
    for (const auto& response : responses) {
        os << response << endl;
    }
}

int main() {
    cout.precision(25);
    const auto requests = ReadRequests();
    const auto responses = ProcessRequests(requests);
    PrintResponses(responses);
}