#pragma once

#include "requests.h"
#include "../../test_runner.h"
#include "json.h"

// Rewrite all tests in this file to use json.h

void TestAddStopRequest() {
    std::istringstream input(std::string(R"({
      "type": "Stop",
      "longitude": 37.20829,
      "name": "Tolstopaltsevo",
      "latitude": 55.611087
    })"));

    auto node = Json::Load(input).GetRoot().AsMap();
        
    auto request = std::make_unique<AddStopRequest>();
    request->ParseFrom(node);
    ASSERT_EQUAL(request->stop.GetName(), "Tolstopaltsevo");
    ASSERT_EQUAL(request->stop.GetLatitude().value(), 55.611087);
    ASSERT_EQUAL(request->stop.GetLongitude().value(), 37.20829);
}

void TestAddBusRequest1() {
    std::istringstream input(std::string(R"({
      "type": "Bus",
      "name": "256",
      "stops": [
      "Tolstopaltsevo",
      "Marushkino"
      ],
      "is_roundtrip": false
    })"));
    //"256: Tolstopaltsevo - Marushkino";
    auto node = Json::Load(input).GetRoot().AsMap();

    auto request = std::make_unique<AddBusRequest>();
    request->ParseFrom(node);
    ASSERT_EQUAL(request->bus_number, "256");
    ASSERT_EQUAL(request->stops.size(), 3);
    ASSERT_EQUAL(request->stops[0], "Tolstopaltsevo");
    ASSERT_EQUAL(request->stops[1], "Marushkino");
    ASSERT_EQUAL(request->stops[2], "Tolstopaltsevo");
}

void TestAddBusRequest2() {
    std::istringstream input(std::string(R"({
        "type": "Bus",
        "name": "750",
        "stops": [
        "Tolstopaltsevo",
        "Marushkino",
        "Rasskazovka",
        "Tolstopaltsevo"
        ],
        "is_roundtrip": true
    })"));

    auto node = Json::Load(input).GetRoot().AsMap();

    auto request = std::make_unique<AddBusRequest>();
    request->ParseFrom(node);

    ASSERT_EQUAL(request->bus_number, "750");
    ASSERT_EQUAL(request->stops.size(), 4);
    ASSERT_EQUAL(request->stops[0], "Tolstopaltsevo");
    ASSERT_EQUAL(request->stops[1], "Marushkino");
    ASSERT_EQUAL(request->stops[2], "Rasskazovka");
    ASSERT_EQUAL(request->stops[3], "Tolstopaltsevo");
}

void TestParseRequest() {
    // Parse bus request 1
    std::istringstream input1(std::string(R"({
        "type": "Bus",
        "name": "256",
        "stops": [
        "Tolstopaltsevo",
        "Marushkino"
        ],
        "is_roundtrip": false
    })"));

    auto node1 = Json::Load(input1).GetRoot().AsMap();
    
    auto request1 = ParseRequest(node1, true);
    ASSERT(request1->type == Request::Type::ADD_BUS);
    auto bus_request1 = static_cast<const AddBusRequest&>(*request1);

    ASSERT_EQUAL(bus_request1.bus_number, "256");
    ASSERT_EQUAL(bus_request1.stops.size(), 3);
    ASSERT_EQUAL(bus_request1.stops[0], "Tolstopaltsevo");
    ASSERT_EQUAL(bus_request1.stops[1], "Marushkino");
    ASSERT_EQUAL(bus_request1.stops[2], "Tolstopaltsevo");

    // Parse bus request 2
    std::istringstream input2(std::string(R"({
        "type": "Bus",
        "name": "750",
        "stops": [
        "Tolstopaltsevo",
        "Marushkino",
        "Rasskazovka",
        "Tolstopaltsevo"
        ],
        "is_roundtrip": true
    })"));
    auto node2 = Json::Load(input2).GetRoot().AsMap();

    auto request2 = ParseRequest(node2, true);
    ASSERT(request2->type == Request::Type::ADD_BUS);
    auto bus_request2 = static_cast<const AddBusRequest&>(*request2);
    ASSERT_EQUAL(bus_request2.bus_number, "750");
    ASSERT_EQUAL(bus_request2.stops.size(), 4);
    ASSERT_EQUAL(bus_request2.stops[0], "Tolstopaltsevo");
    ASSERT_EQUAL(bus_request2.stops[1], "Marushkino");
    ASSERT_EQUAL(bus_request2.stops[2], "Rasskazovka");
    ASSERT_EQUAL(bus_request2.stops[3], "Tolstopaltsevo");

    // Parse stop request
    std:: istringstream input3(std::string(R"({
        "type": "Stop",
        "longitude": 37.20829,
        "name": "Tolstopaltsevo",
        "latitude": 55.611087
    })"));

    auto node3 = Json::Load(input3).GetRoot().AsMap();

    auto request3 = ParseRequest(node3, true);
    ASSERT(request3->type == Request::Type::ADD_STOP);
    auto stop_request = static_cast<const AddStopRequest&>(*request3);
    ASSERT_EQUAL(stop_request.stop.GetName(), "Tolstopaltsevo");
    ASSERT_EQUAL(stop_request.stop.GetLatitude().value(), 55.611087);
    ASSERT_EQUAL(stop_request.stop.GetLongitude().value(), 37.20829);
}

void TestReadRequests() {
    /*
    R"(3\
Stop Tolstopaltsevo: 55.611087, 37.20829
Stop Marushkino: 55.595884, 37.209755
Bus 256: Tolstopaltsevo - Marushkino)"
    */
    std::stringstream is(R"({
        "base_requests": [
            {
                "type": "Stop",
                "longitude": 37.20829,
                "name": "Tolstopaltsevo",
                "latitude": 55.611087
            },
            {
                "type": "Stop",
                "longitude": 37.209755,
                "name": "Marushkino",
                "latitude": 55.595884
            },
            {
                "type": "Bus",
                "name": "256",
                "stops": [
                    "Tolstopaltsevo",
                    "Marushkino"
                ],
                "is_roundtrip": false
            }
        ]
    })");
    Json::Document doc = Json::Load(is);
    auto requests = ReadRequests(doc.GetRoot().AsMap().at("base_requests").AsArray(), true);
    ASSERT_EQUAL(requests.size(), 3);

    ASSERT(requests[0]->type == Request::Type::ADD_STOP);
    auto request0 = static_cast<const AddStopRequest&>(*requests[0]);
    ASSERT_EQUAL(request0.stop.GetName(), "Tolstopaltsevo");
    ASSERT_EQUAL(request0.stop.GetLatitude().value(), 55.611087);
    ASSERT_EQUAL(request0.stop.GetLongitude().value(), 37.20829);

    ASSERT(requests[1]->type == Request::Type::ADD_STOP);
    auto request1 = static_cast<const AddStopRequest&>(*requests[1]);
    ASSERT_EQUAL(request1.stop.GetName(), "Marushkino");
    ASSERT_EQUAL(request1.stop.GetLatitude().value(), 55.595884);
    ASSERT_EQUAL(request1.stop.GetLongitude().value(), 37.209755);

    ASSERT(requests[2]->type == Request::Type::ADD_BUS);
    auto request2 = static_cast<const AddBusRequest&>(*requests[2]);
    ASSERT_EQUAL(request2.bus_number, "256");
    ASSERT_EQUAL(request2.stops.size(), 3);
    ASSERT_EQUAL(request2.stops[0], "Tolstopaltsevo");
    ASSERT_EQUAL(request2.stops[1], "Marushkino");
    ASSERT_EQUAL(request2.stops[2], "Tolstopaltsevo");
    
}

void TestAllRequests() {
    TestRunner tr;
    RUN_TEST(tr, TestAddStopRequest);
    RUN_TEST(tr, TestAddBusRequest1);
    RUN_TEST(tr, TestAddBusRequest2);
    RUN_TEST(tr, TestParseRequest);
    RUN_TEST(tr, TestReadRequests);
}