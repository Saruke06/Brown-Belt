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

optional<Request::Type> ConvertRequestTypeFromString(string_view type_str, bool is_add_request) {
    if (is_add_request) {
        if (type_str == "Bus") {
            return Request::Type::ADD_BUS;
        } else if (type_str == "Stop") {
            return Request::Type::ADD_STOP;
        }
    } else {
        if (type_str == "Bus") {
            return Request::Type::OUT_BUS;
        }
    }
    return nullopt;
}

RequestHolder ParseRequest(string_view request_str) {
    bool is_add_request = request_str.find(":") != string::npos;
    const auto request_type = ConvertRequestTypeFromString(ReadToken(request_str), is_add_request);
    if (!request_type) {
        return nullptr;
    }

    RequestHolder request = Request::Create(*request_type);
    if (request) {
        request->ParseFrom(request_str);
    }
    return request;
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
    const size_t output_requests = ReadNumberOnLine<size_t>(input);
    for (size_t i = 0; i < output_requests; ++i) {
        std::string update_str;
        getline(input, update_str);
        if (auto request = ParseRequest(update_str)) {
            updates.push_back(std::move(request));
        }
    }
    return updates;
}

vector<string> ProcessRequests(const vector<RequestHolder>& requests) {
    vector<string> responses;
    TransportDatabase db;

    for (const auto& request_holder : requests) {
        if (request_holder->type == Request::Type::OUT_BUS) {
            const auto& request = static_cast<const BusRequest&>(*request_holder);
            responses.push_back(request.Process(db));
        } else {
            const auto& request = static_cast<const ModifyRequest&>(*request_holder);
            request.Process(db);
        }
    }

    return responses;
}

void PrintResponses(const vector<string>& responses, ostream& output = cout) {
    for (const string& response : responses) {
        output << response << '\n';
    }
}

int main() {
    TransportDatabase db;

    const auto requests = ReadRequests();
    const auto responses = ProcessRequests(requests);
    PrintResponses(responses);

    return 0;
}