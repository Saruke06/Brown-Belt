#pragma once

#include <string>
#include <memory>

#include "transport_db.h"

struct Request;
using RequestHolder = std::unique_ptr<Request>;

struct Request {
    enum class Type {
        STOP,
        BUS
    };

    Request(Type type) : type(type) {}
    static RequestHolder Create(Request::Type type);
    virtual void ParseFrom(std::string_view input) = 0;
    virtual ~Request() = default;

    const Type type;
};

struct ModifyRequest : Request {
    using Request::Request;
    virtual void Process(const TransportDatabase& db) const = 0;
};

struct AddBusRequest : ModifyRequest {
    AddBusRequest() : ModifyRequest(Type::BUS) {}

    void ParseFrom(std::string_view input) override {
        // BUS 256
    }

    void Process(TransportDatabase& db) const override {
        // db.AddBus();
    }
};

struct AddStopRequest : ModifyRequest {
    AddStopRequest() : ModifyRequest(Type::STOP) {}

    void ParseFrom(std::string_view input) override {
        // STOP 256
    }

    void Process(TransportDatabase& db) const override {
        // db.AddStop();
    }
};

template <typename ResultType>
struct ReadRequest : Request {
    using Request::Request;
    virtual ResultType Process(const TransportDatabase& db) const = 0;
};

struct BusRequest : ReadRequest<std::string> {
    BusRequest() : ReadRequest(Type::BUS) {}

    void ParseFrom(std::string_view input) override {
        // BUS 256
    }

    std::string Process(const TransportDatabase& db) const override {
        // return db.GetBus();
    }
};
