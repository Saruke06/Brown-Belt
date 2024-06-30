#pragma once

#include "transport_db.h"
#include "requests.h"
#include "../../test_runner.h"

void TestComputeDistance() {
    Stop Tolstopaltsevo("Tolstopaltsevo");
    Tolstopaltsevo.SetLatitude(55.611087); // rad = 0.9705965687 sin = 0.8252228066 cos = 0.5648073296
    Tolstopaltsevo.SetLongitude(37.20829); // rad = 0.6494071695 sin = 0.6047143567 cos = 0.7964424316

    Stop Marushkino("Marushkino");
    Marushkino.SetLatitude(55.595884); // rad = 0.9703312263 sin = 0.8250729102 cos = 0.5650262763
    Marushkino.SetLongitude(37.209755); // rad = 0.6494327385 sin = 0.6047347208 cos = 0.7964269693

    Stop Moscow("Moscow");
    Moscow.SetLatitude(55.751244);
    Moscow.SetLongitude(37.618423);

    Stop SaintPetersburg("Saint Petersburg");
    SaintPetersburg.SetLatitude(59.93863);
    SaintPetersburg.SetLongitude(30.31413);

    double distance1 = TransportDatabase::ComputeDistance(Tolstopaltsevo, Marushkino);
    // double distance2 = TransportDatabase::ComputeDistance(Moscow, SaintPetersburg);

    //std::cerr << distance1 << ' ' << distance2 << std::endl;
}

void TestAddStopDB() {
    TransportDatabase db;
    // Add one stop to db and check if it exists
    auto request_holder = ParseRequest("Stop Tolstopaltsevo: 55.611087, 37.20829");
    const auto& request = static_cast<const ModifyRequest&>(*request_holder);

    request.Process(db);

    std::string stop_info = db.GetStop("Tolstopaltsevo");
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
    std::string stop_info = db.GetStop("Tolstopaltsevo");
    const std::string expected = "Stop Tolstopaltsevo exists with latitude 55.611087 and longitude 37.208290";
    ASSERT_EQUAL(expected, stop_info);

    stop_info = db.GetStop("Marushkino");
    const std::string expected2 = "Stop Marushkino exists with latitude unknown and longitude unknown";
    ASSERT_EQUAL(expected2, stop_info);

    // 4.
    request_holder = ParseRequest("Stop Marushkino: 55.595884, 37.209755");
    const auto& request3 = static_cast<const ModifyRequest&>(*request_holder);
    request3.Process(db);

    // 5.
    stop_info = db.GetStop("Tolstopaltsevo");
    ASSERT_EQUAL(expected, stop_info);

    stop_info = db.GetStop("Marushkino");
    const std::string expected3 = "Stop Marushkino exists with latitude 55.595884 and longitude 37.209755";
    ASSERT_EQUAL(expected3, stop_info);

    // 6.
    stop_info = db.GetStop("Rasskazovka");
    const std::string expected4 = "NO stop Rasskazovka in the DB";
    ASSERT_EQUAL(expected4, stop_info);

}

void TestAllTransportDB() {
    TestRunner tr;
    // RUN_TEST(tr, TestComputeDistance);
    RUN_TEST(tr, TestAddStopDB);
    RUN_TEST(tr, TestAddBusRouteDB);
}