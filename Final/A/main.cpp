#include <iostream>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>

#include "string_parses.h"
#include "requests.h"
#include "transport_db.h"

using namespace std;

optional<Request::Type> ConvertRequestTypeFromString(string_view type_str) {
    if (type_str == "Bus") {
        return Request::Type::BUS;
    } else if (type_str == "Stop") {
        return Request::Type::STOP;
    }
    return nullopt;
}

RequestHolder ParseRequest(string_view request_str) {
    const auto request_type = ConvertRequestTypeFromString(ReadToken(request_str));
    bool is_add_request = request_type.find(":") != string::npos;
    if (is_add_request) {
        if (request_type == "Stop") {
            return make_unique<AddStopRequest>();
        } else if (request_type == "Bus") {
            return make_unique<AddBusRequest>();
        }
    } else {
        if (request_type == "Bus") {
            return make_unique<BusRequest>();
        }
    }
    return nullptr;
}

vector<RequestHolder> ReadRequests(istream& input = cin) {
    const size_t updates_count = ReadNumberOnLine<size_t>(input);
    vector<RequestHolder> updates(updates_count);
    for (size_t i = 0; i < updates_count; ++i) {
        std::string update_str;
        getline(input, update_str);
        if (auto request = ParseRequest(update_str)) {
            updates.push_back(std::move(request));
        }
    }
    return updates;
}

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