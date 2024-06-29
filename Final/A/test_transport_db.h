#pragma once

#include "transport_db.h"
#include "../../test_runner.h"

void TestComputeDistance() {
    std::cerr.precision(10);
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

    if (!request_holder)
    {
        std::cout << "sosesh" << std::endl;
    } else {
        request.Process(db);
    }

    std::string stop_info = db.GetStop("Tolstopaltsevo");
    const std::string expected = "Stop Tolstopaltsevo exists with latitude 55.611087 and longitude 37.20829";
    ASSERT_EQUAL(expected, stop_info);
}

// TODO: В аутпуте TestAddStop 4 знака после точки, а должно быть 6
// TODO: Написать Тест для добавлния Автобуса и координат к его остановкам

void TestAddBusRouteDB() {
    
}

void TestAllTransportDB() {
    TestRunner tr;
    // RUN_TEST(tr, TestComputeDistance);
    RUN_TEST(tr, TestAddStopDB);
}