#include <iostream>
#include <ctime>
#include <stdexcept>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <memory>

using namespace std;

struct Date {
    int day_;
    int month_;
    int year_;
    Date() = default;
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
};

istream& operator>>(istream& is, Date& date) {
    char c1, c2;
    if (is >> date.year_ >> c1 >> date.month_ >> c2 >> date.day_ &&
        c1 == '-' && c2 == '-') {
        return is;
    } else {
        throw invalid_argument("Wrong date format");
    }
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
    virtual void Create() = 0;
    virtual ~Request() = default;

    const Type type;
};


pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimiter = " ") {
    const size_t pos = s.find(delimiter);
    if (pos == s.npos) {
        return {s, nullopt};
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