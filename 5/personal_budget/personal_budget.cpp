#include <iostream>
#include <ctime>
#include <stdexcept>
#include <vector>
#include <string>

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

enum class RequestType {
    COMPUTE_INCOME,
    EARN,
    PAY_TAX
};

struct Request {
    RequestType type;
    Date from;
    Date to;
    int value = 0;
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
        throw invalid_argument("Unknown query type");
    }
    return query;
};

template <typename Number>
Number ReadNumberOnLine(istream& is = cin) {
    Number number;
    is >> number;
    string dummy;
    getline(is, dummy);
    return number;
}

Request ParseRequest(string_view request_str) {

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