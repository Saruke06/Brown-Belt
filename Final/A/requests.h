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
