#pragma once

#include "requests.h"
#include "../../test_runner.h"
#include "json.h"

// Rewrite all tests in this file to use json.h

void TestAddStopRequest() {
    std::istringstream node_str(std::string(R"({
      "type": "Stop",
      "longitude": 37.20829,
      "name": "Tolstopaltsevo",
      "latitude": 55.611087
    })"));

    auto node = Json::Load(node_str).GetRoot().AsMap();
        
    auto request = std::make_unique<AddStopRequest>();
    request->ParseFrom(node);
    ASSERT_EQUAL(request->stop.GetName(), "Tolstopaltsevo");
    ASSERT_EQUAL(request->stop.GetLatitude().value(), 55.611087);
    ASSERT_EQUAL(request->stop.GetLongitude().value(), 37.20829);
}

void TestAddBusRequest1() {
    std::string_view input = "256: Tolstopaltsevo - Marushkino";
    auto request = std::make_unique<AddBusRequest>();
    request->ParseFrom(input);
    ASSERT_EQUAL(request->bus_number, "256");
    ASSERT_EQUAL(request->stops.size(), 3);
    ASSERT_EQUAL(request->stops[0], "Tolstopaltsevo");
    ASSERT_EQUAL(request->stops[1], "Marushkino");
    ASSERT_EQUAL(request->stops[2], "Tolstopaltsevo");
}

// void TestAddBusRequest2() {
//     std::string_view input = "750: Tolstopaltsevo > Marushkino > Rasskazovka > Tolstopaltsevo";
//     auto request = std::make_unique<AddBusRequest>();
//     request->ParseFrom(input);

//     ASSERT_EQUAL(request->bus_number, "750");
//     ASSERT_EQUAL(request->stops.size(), 4);
//     ASSERT_EQUAL(request->stops[0], "Tolstopaltsevo");
//     ASSERT_EQUAL(request->stops[1], "Marushkino");
//     ASSERT_EQUAL(request->stops[2], "Rasskazovka");
//     ASSERT_EQUAL(request->stops[3], "Tolstopaltsevo");
// }

// void TestParseRequest() {
//     // Parse bus request 1
//     std::string_view input = "Bus 256: Tolstopaltsevo - Marushkino";
//     auto request = ParseRequest(input);
//     ASSERT(request->type == Request::Type::ADD_BUS);
//     auto bus_request = static_cast<AddBusRequest*>(request.get());
//     ASSERT_EQUAL(bus_request->bus_number, "256");
//     ASSERT_EQUAL(bus_request->stops.size(), 3);
//     ASSERT_EQUAL(bus_request->stops[0], "Tolstopaltsevo");
//     ASSERT_EQUAL(bus_request->stops[1], "Marushkino");

//     // Parse bus request 2
//     input = "Bus 750: Tolstopaltsevo > Marushkino > Rasskazovka > Tolstopaltsevo";
//     request = ParseRequest(input);
//     ASSERT(request->type == Request::Type::ADD_BUS);
//     auto bus_request2 = static_cast<AddBusRequest*>(request.get());
//     ASSERT_EQUAL(bus_request2->bus_number, "750");
//     ASSERT_EQUAL(bus_request2->stops.size(), 4);
//     ASSERT_EQUAL(bus_request2->stops[0], "Tolstopaltsevo");
//     ASSERT_EQUAL(bus_request2->stops[1], "Marushkino");
//     ASSERT_EQUAL(bus_request2->stops[2], "Rasskazovka");
//     ASSERT_EQUAL(bus_request2->stops[3], "Tolstopaltsevo");

//     // Parse stop request
//     input = "Stop Tolstopaltsevo: 55.611087, 37.20829";
//     request = ParseRequest(input);
//     ASSERT(request->type == Request::Type::ADD_STOP);
//     auto stop_request = static_cast<AddStopRequest*>(request.get());
//     ASSERT_EQUAL(stop_request->stop.GetName(), "Tolstopaltsevo");
//     ASSERT_EQUAL(stop_request->stop.GetLatitude().value(), 55.611087);
//     ASSERT_EQUAL(stop_request->stop.GetLongitude().value(), 37.20829);
// }

// void TestReadRequests() {
//     std::stringstream is(R"(3\
// Stop Tolstopaltsevo: 55.611087, 37.20829
// Stop Marushkino: 55.595884, 37.209755
// Bus 256: Tolstopaltsevo - Marushkino)");
//     auto requests = ReadRequests(is);
//     ASSERT_EQUAL(requests.size(), 3);
//     ASSERT(requests[0]->type == Request::Type::ADD_STOP);
//     ASSERT(requests[1]->type == Request::Type::ADD_STOP);
//     ASSERT(requests[2]->type == Request::Type::ADD_BUS);
// }

void TestAllRequests() {
    TestRunner tr;
    RUN_TEST(tr, TestAddStopRequest);
    // RUN_TEST(tr, TestAddBusRequest1);
    // RUN_TEST(tr, TestAddBusRequest2);
    // RUN_TEST(tr, TestParseRequest);
    // RUN_TEST(tr, TestReadRequests);
}