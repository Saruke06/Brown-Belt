#pragma once

#include "transport_db.h"
#include "requests.h"
#include "../../test_runner.h"

void TestGetBus() {
    std::stringstream is1(R"(4\
Stop Tolstopaltsevo: 55.611087, 37.20829
Stop Marushkino: 55.595884, 37.209755
Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka
Stop Rasskazovka: 55.632761, 37.333324)");
    std::stringstream is2(R"(1\
Bus 750)");
    std::ostringstream os;
    TransportDatabase db;
    const auto modify_requests = ReadRequests(is1);
    ProcessModifyRequests(&db, modify_requests);
    // Check if  all stops are added correctly
    std::string stop_info = db.GetStopInfo("Tolstopaltsevo");
    const std::string expected = "Stop Tolstopaltsevo exists with latitude 55.611087 and longitude 37.208290";
    ASSERT_EQUAL(expected, stop_info);

    stop_info = db.GetStopInfo("Marushkino");
    const std::string expected2 = "Stop Marushkino exists with latitude 55.595884 and longitude 37.209755";
    ASSERT_EQUAL(expected2, stop_info);

    stop_info = db.GetStopInfo("Rasskazovka");
    const std::string expected3 = "Stop Rasskazovka exists with latitude 55.632761 and longitude 37.333324";
    ASSERT_EQUAL(expected3, stop_info);


    const auto read_requests = ReadRequests(is2);
    const auto responses = ProcessRequests(db, read_requests);
    PrintResponses(responses, os);
    const std::string expected4 = "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length\n";
    ASSERT_EQUAL(os.str(), expected4);
}

void TestAddStopDB() {
    TransportDatabase db;
    // Add one stop to db and check if it exists
    auto request_holder = ParseRequest("Stop Tolstopaltsevo: 55.611087, 37.20829");
    const auto& request = static_cast<const ModifyRequest&>(*request_holder);

    request.Process(db);

    std::string stop_info = db.GetStopInfo("Tolstopaltsevo");
    const std::string expected = "Stop Tolstopaltsevo exists with latitude 55.611087 and longitude 37.208290";
    ASSERT_EQUAL(expected, stop_info);
}

void TestAddBusRouteDB() {
    // 1. Add one stop;
    // 2. Add Bus with route contains two stops including the first one
    // 3. Check both stops for coordinates
    // 4. Add Second stop
    // 5. Check both stops for coordinates
    // 6. Check random stop that doesn't exist

    TransportDatabase db;
    // 1.
    auto request_holder = ParseRequest("Stop Tolstopaltsevo: 55.611087, 37.20829");
    const auto& request = static_cast<const ModifyRequest&>(*request_holder);
    request.Process(db);

    // 2.
    request_holder = ParseRequest("Bus 256: Tolstopaltsevo - Marushkino");
    const auto& request2 = static_cast<const ModifyRequest&>(*request_holder);
    request2.Process(db);

    // 3.
    std::string stop_info = db.GetStopInfo("Tolstopaltsevo");
    const std::string expected = "Stop Tolstopaltsevo exists with latitude 55.611087 and longitude 37.208290";
    ASSERT_EQUAL(expected, stop_info);

    stop_info = db.GetStopInfo("Marushkino");
    const std::string expected2 = "Stop Marushkino exists with latitude unknown and longitude unknown";
    ASSERT_EQUAL(expected2, stop_info);

    // 4.
    request_holder = ParseRequest("Stop Marushkino: 55.595884, 37.209755");
    const auto& request3 = static_cast<const ModifyRequest&>(*request_holder);
    request3.Process(db);

    // 5.
    stop_info = db.GetStopInfo("Tolstopaltsevo");
    ASSERT_EQUAL(expected, stop_info);

    stop_info = db.GetStopInfo("Marushkino");
    const std::string expected3 = "Stop Marushkino exists with latitude 55.595884 and longitude 37.209755";
    ASSERT_EQUAL(expected3, stop_info);

    // 6.
    stop_info = db.GetStopInfo("Rasskazovka");
    const std::string expected4 = "NO stop Rasskazovka in the DB";
    ASSERT_EQUAL(expected4, stop_info);

}

void TestAllTransportDB() {
    TestRunner tr;
    RUN_TEST(tr, TestGetBus);
    RUN_TEST(tr, TestAddStopDB);
    RUN_TEST(tr, TestAddBusRouteDB);
}