#pragma once

#include <string>
#include <memory>

#include "transport_db.h"

struct Request;
using RequestHolder = std::unique_ptr<Request>;

struct Request {
    enum class Type {
        ADD_STOP,
        ADD_BUS,
        OUT_BUS
    };

    Request(Type type) : type(type) {}
    static RequestHolder Create(Request::Type type);
    virtual void ParseFrom(std::string_view input) = 0;
    virtual ~Request() = default;

    const Type type;
};

struct ModifyRequest : Request {
    using Request::Request;
    virtual void Process(TransportDatabase& db) const = 0;
};

struct AddBusRequest : ModifyRequest {
    AddBusRequest() : ModifyRequest(Type::ADD_BUS) {}

    void ParseFrom(std::string_view input) override {
        bus_number = std::string(ReadToken(input, ": "));
        stops = ParseStops(input);
    }

    void Process(TransportDatabase& db) const override {
        db.AddBus(std::move(bus_number), std::move(stops));
    }

    std::string bus_number;
    std::vector<Stop> stops;
};

struct AddStopRequest : ModifyRequest {
    AddStopRequest() : ModifyRequest(Type::ADD_STOP) {}

    void ParseFrom(std::string_view input) override {
        // input = "X: latitude, longitude"
        stop = Stop::ParseFrom(input);
    }

    void Process(TransportDatabase& db) const override {
        // Add stop to db
        db.AddStop(std::move(stop));
    }

    Stop stop;
};

template <typename ResultType>
struct ReadRequest : Request {
    using Request::Request;
    virtual ResultType Process(const TransportDatabase& db) const = 0;
};

struct BusRequest : ReadRequest<std::string> {
    BusRequest() : ReadRequest(Type::OUT_BUS) {}

    void ParseFrom(std::string_view input) override {
        // input = "X"
        bus_number = std::string(input);
    }

    std::string Process(const TransportDatabase& db) const override {
        // return db.GetBus();
        return db.GetBusInfo(bus_number);
    }

    std::string bus_number;
};

RequestHolder Request::Create(Request::Type type) {
    switch (type) {
        case Type::ADD_STOP:
            return std::make_unique<AddStopRequest>();
        case Type::ADD_BUS:
            return std::make_unique<AddBusRequest>();
        case Type::OUT_BUS:
            return std::make_unique<BusRequest>();
        default:
            return nullptr;
    }
}

std::optional<Request::Type> ConvertRequestTypeFromString(std::string_view type_str, bool is_add_request) {
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
    return std::nullopt;
}

RequestHolder ParseRequest(std::string_view request_str) {
    bool is_add_request = request_str.find(":") != std::string::npos;
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

std::vector<RequestHolder> ReadRequests(std::istream& input = std::cin) {
    const size_t request_count = ReadNumberOnLine<size_t>(input);
    std::vector<RequestHolder> updates;
    updates.reserve(request_count);

    for (size_t i = 0; i < request_count; ++i) {
        std::string request_str;
        getline(input, request_str);
        if (auto request = ParseRequest(request_str)) {
            updates.push_back(std::move(request));
        }
    }
    return updates;
}

void ProcessModifyRequests(TransportDatabase* db = nullptr, const std::vector<RequestHolder>& requests = {}) {
    if (!db) {
        throw std::invalid_argument("db is nullptr");
    }
    for (const auto& request_holder : requests) {
        const auto& request = static_cast<const ModifyRequest&>(*request_holder);
        request.Process(*db);
    }
}

std::vector<std::string> ProcessRequests(const TransportDatabase& db, const std::vector<RequestHolder>& requests) {
    std::vector<std::string> responses;

    for (const auto& request_holder : requests) {
        if (request_holder->type == Request::Type::OUT_BUS) {
            const auto& request = static_cast<const BusRequest&>(*request_holder);
            responses.push_back(request.Process(db));
        }
        // } else {
        //     const auto& request = static_cast<const ModifyRequest&>(*request_holder);
        //     request.Process(db);
        // }
    }

    return responses;
}

void PrintResponses(const std::vector<std::string>& responses, std::ostream& output = std::cout) {
    for (const std::string& response : responses) {
        output << response << '\n';
    }
}