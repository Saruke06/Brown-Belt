#pragma once

#include <string>
#include <memory>
#include <variant>

#include "transport_db.h"
#include "json.h"

struct Request;
using RequestHolder = std::unique_ptr<Request>;

struct Request {
    enum class Type {
        ADD_STOP,
        ADD_BUS,
        OUT_BUS,
        OUT_STOP
    };

    Request(Type type) : type(type) {}
    static RequestHolder Create(Request::Type type);
    virtual void ParseFrom(const Json::Node& request_node) = 0;
    virtual ~Request() = default;

    const Type type;
};

struct ModifyRequest : Request {
    using Request::Request;
    virtual void Process(TransportDatabase& db) const = 0;
    virtual ~ModifyRequest() = default;
};

struct AddBusRequest : ModifyRequest {
    AddBusRequest() : ModifyRequest(Type::ADD_BUS) {}
    virtual ~AddBusRequest() = default;

    std::vector<std::string> NodesToStrings(const std::vector<Json::Node>& nodes, bool is_roundtrip) const {
        std::vector<std::string> result;
        for (const Json::Node& node : nodes) {
            result.push_back(node.AsString());
        }

        if (!is_roundtrip) {
            for (size_t i = result.size() - 1; i > 0; --i) {
                result.push_back(result[i - 1]);
            }
        }
        return result;
    }

    void ParseFrom(const Json::Node& request_node) override {
        bus_number = request_node.AsMap().at("name").AsString();
        bool is_roundtrip = request_node.AsMap().at("is_roundtrip").AsBool();
        
        stops = NodesToStrings(request_node.AsMap().at("stops").AsArray(), is_roundtrip);
    }

    void Process(TransportDatabase& db) const override {
        db.AddBus(std::move(bus_number), std::move(stops));
    }

    std::string bus_number;
    std::vector<std::string> stops;
};

struct AddStopRequest : ModifyRequest {
    AddStopRequest() : ModifyRequest(Type::ADD_STOP) {}
    virtual ~AddStopRequest() = default;

    void ParseFrom(const Json::Node& request_node) override {
        stop = Stop::ParseFrom(request_node);
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
    virtual ~ReadRequest() = default;
};

struct BusRequest : ReadRequest<std::string> {
    BusRequest() : ReadRequest(Type::OUT_BUS) {}
    virtual ~BusRequest() = default;

    void ParseFrom(const Json::Node& request_node) override {
        // input = "X"
        id = request_node.AsMap().at("id").AsInt();
        bus_number = request_node.AsMap().at("name").AsString();
    }

    std::string Process(const TransportDatabase& db) const override {
        return db.GetBusInfo(bus_number, id);
    }

    std::string bus_number;
    int id = 0;
};

struct StopRequest : ReadRequest<std::string> {
    StopRequest() : ReadRequest(Type::OUT_STOP) {}
    virtual ~StopRequest() = default;

    void ParseFrom(const Json::Node& request_node) override {
        // input = "X"
        id = request_node.AsMap().at("id").AsInt();
        stop_name = request_node.AsMap().at("name").AsString();
    }

    std::string Process(const TransportDatabase& db) const override {
        return db.GetStopInfo(stop_name, id);
    }

    std::string stop_name;
    int id = 0;
};

RequestHolder Request::Create(Request::Type type) {
    switch (type) {
        case Type::ADD_STOP:
            return std::make_unique<AddStopRequest>();
        case Type::ADD_BUS:
            return std::make_unique<AddBusRequest>();
        case Type::OUT_BUS:
            return std::make_unique<BusRequest>();
        case Type::OUT_STOP:
            return std::make_unique<StopRequest>();
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
        } else if (type_str == "Stop") {
            return Request::Type::OUT_STOP;
        }
    }
    return std::nullopt;
}

RequestHolder ParseRequest(const Json::Node& request_node, bool is_add_request) {
    const auto request_type = ConvertRequestTypeFromString(request_node.AsMap().at("type").AsString(), is_add_request);
    if (!request_type) {
        return nullptr;
    }

    RequestHolder request = Request::Create(*request_type);
    if (request) {
        request->ParseFrom(request_node);
    }
    return request;
}

std::vector<RequestHolder> ReadRequests(const std::vector<Json::Node>& requests, bool is_add_request) {
    const size_t request_count = requests.size();
    std::vector<RequestHolder> updates;
    updates.reserve(request_count);

    for (size_t i = 0; i < request_count; ++i) {
        if (auto request = ParseRequest(requests[i], is_add_request)) {
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

std::vector<Json::Node> ProcessRequestsJson(const TransportDatabase& db, const std::vector<RequestHolder>& requests) {
    std::vector<Json::Node> responses;

    for (const auto& request_holder : requests) {
        switch(request_holder->type) {
            case Request::Type::OUT_BUS:
            case Request::Type::OUT_STOP: {
                const auto& request = static_cast<const ReadRequest<Json::Node>&>(*request_holder);
                responses.push_back(request.Process(db));
                break;
            }
            default:
                throw std::invalid_argument("Unknown request type");
        }
    }

    return responses;
}

std::vector<std::string> ProcessRequests(const TransportDatabase& db, const std::vector<RequestHolder>& requests) {
    std::vector<std::string> responses;

    for (const auto& request_holder : requests) {
        switch(request_holder->type) {
            case Request::Type::OUT_BUS:
            case Request::Type::OUT_STOP: {
                const auto& request = static_cast<const ReadRequest<std::string>&>(*request_holder);
                responses.push_back(request.Process(db));
                break;
            }
            default:
                throw std::invalid_argument("Unknown request type");
        }
    }

    return responses;
}

// void PrintResponsesJson(const std::vector<Json::Node>& responses, std::ostream& output = std::cout) {
//     output << responses;
//     // for (const auto& response : responses) {
//     //     output << response << std::endl;
//     // }
// }

void PrintResponses(const std::vector<std::string>& responses, std::ostream& output = std::cout) {
    // вывести все элементы векторатора respo вмежду э, каждый элемент выводить на новой строке
    // между элементами вставить запятую и пробел
    bool first = true;
    output << "[\n";
    for (const auto& response : responses) {
        if (first) {
            output << response;
            first = false;
        } else {
            output << ", \n" << response;
        }
    }
    output << "\n]";
}