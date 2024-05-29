#include <iostream>
#include <ctime>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <optional>
#include <sstream>

using namespace std;

pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimiter) {
    const size_t pos = s.find(delimiter);
    if (pos == s.npos) {
        return {s, nullopt};
    } else {
        return {s.substr(0, pos), s.substr(pos + delimiter.length())};
    }
}

pair<string_view, string_view> SplitTwo(string_view s, string_view delimiter) {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    return {lhs, rhs_opt.value_or("")};
}

string_view ReadToken(string_view& str, string_view delimiter) {
    const auto [lhs, rhs] = SplitTwo(str, delimiter);
    str = rhs;
    return lhs;
}

int ConvertToInt(string_view str) {
    size_t pos;
    const int result = stoi(string(str), &pos);
    if (pos != str.length()) {
        std::stringstream error;
        error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
        throw invalid_argument(error.str());
    }
    return result;
}

template <typename Number>
void ValidateBounds(Number number_to_check, Number min_value, Number max_value) {
    if (number_to_check < min_value || number_to_check > max_value) {
        std::stringstream error;
        error << number_to_check << " is out of [" << min_value << ", " << max_value << "]";
        throw invalid_argument(error.str());
    }
}

struct IndexSegment {
    size_t left;
    size_t right;

    size_t length() const {
        return right - left;
    }
    bool empty() const {
        return length() == 0;;
    }

    bool Contains(IndexSegment other) const {
        return left <= other.left && right >= other.right;
    }
};

IndexSegment IntersectSegments(IndexSegment lhs, IndexSegment rhs) {
    const size_t left = max(lhs.left, rhs.left);
    const size_t right = min(lhs.right, rhs.right);
    return {left, max(left, right)};
}

bool AreSegmentsIntersected(IndexSegment lhs, IndexSegment rhs) {
    return !(lhs.right <= rhs.left || rhs.right <= lhs.left);
}

struct BulkMoneyAdder {
    double delta = 0.0;
};

constexpr uint8_t TAX_PERCENTAGE = 13;

struct BulkTaxApplier {
    static constexpr double FACTOR = 1.0 - TAX_PERCENTAGE / 100.0;
    uint32_t count = 0;

    double ComputeFactor() const {
        return pow(FACTOR, count);
    }
};

class BulkLinearUpdater {
public:
    BulkLinearUpdater() = default;

};

class Date {
public:
    static Date ParseFrom(string_view str) {
        const int year = ConvertToInt(ReadToken(str, "-"));
        const int month = ConvertToInt(ReadToken(str, "-"));
        ValidateBounds(month, 1, 12);
        const int day = ConvertToInt(str);
        ValidateBounds(day, 1, 31);
        return {year, month, day};
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
    int year_;
    int month_;
    int day_;
    
    Date(int year, int month, int day)
        : day_(day), month_(month), year_(year) {}
};

int ComputeDaysDiff(const Date& date_to, const Date& date_from) {
    const time_t timestamp_to = date_to.AsTimestamp();
    const time_t timestamp_from = date_from.AsTimestamp();
    static const int SECONDS_IN_DAY = 60 * 60 * 24;
    return (timestamp_to - timestamp_from) / SECONDS_IN_DAY;
}

static const Date START_DATE = Date::ParseFrom("2000-01-01");
static const Date END_DATE = Date::ParseFrom("2100-01-01");
static const size_t DAY_COUNT = ComputeDaysDiff(START_DATE, END_DATE); 

size_t ComputeDayIndex(const Date& date) {
    return ComputeDaysDiff(date, START_DATE);
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