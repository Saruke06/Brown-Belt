#pragma once

#include "transport_db.h"
#include "requests.h"
#include "../../test_runner.h"
#include "json.h"

void TestGetBus() {
    using namespace Json;
    std::istringstream is(R"({
        "base_requests": [
            {
                "type": "Stop",
                "road_distances": {
                    "Marushkino": 390000
                },
                "longitude": 37.20829,
                "name": "Tolstopaltsevo",
                "latitude": 55.611087
            },
            {
                "type": "Stop",
                "road_distances": {
                    "Rasskazovka": 990000
                },
                "longitude": 37.209755,
                "name": "Marushkino",
                "latitude": 55.595884
            },
            {
                "type": "Bus",
                "name": "750",
                "stops": [
                    "Tolstopaltsevo",
                    "Marushkino",
                    "Rasskazovka"
                ],
                "is_roundtrip": false
            },
            {
                "type": "Stop",
                "longitude": 37.333324,
                "name": "Rasskazovka",
                "latitude": 55.632761
            }
        ],
        "stat_requests": [
            {
                "type": "Bus",
                "name": "750",
                "id": 1971
            }
        ]
    })");
    std::ostringstream os;
    TransportDatabase db;
    Document doc = Load(is);
    const auto modify_requests = ReadRequests(doc.GetRoot().AsMap().at("base_requests").AsArray(), true);
    ProcessModifyRequests(&db, modify_requests);
    // Check if  all stops are added correctly
    Node stop_info = db.GetStopInfo("Tolstopaltsevo", 1234);
    const Node expected = Node(std::map<std::string, Node>{
            {"buses", Node(std::vector<Node>{"750"s})},
            {"request_id", Node(1234)}
        });
    ASSERT_EQUAL(expected, stop_info);

    stop_info = db.GetStopInfo("Marushkino", 4321);
    const Node expected2 = Node(std::map<std::string, Node>{
            {"buses", Node(std::vector<Node>{"750"s})},
            {"request_id", Node(4321)}
        });
    ASSERT_EQUAL(expected2, stop_info);

    stop_info = db.GetStopInfo("Rasskazovka", 228);
    const Node expected3 = Node(std::map<std::string, Node>{
            {"buses", Node(std::vector<Node>{"750"s})},
            {"request_id", Node(228)}
        });
    ASSERT_EQUAL(expected3, stop_info);


    const auto read_requests = ReadRequests(doc.GetRoot().AsMap().at("stat_requests").AsArray(), false);
    const auto responses = ProcessRequests(db, read_requests);
    PrintResponses(responses, os);
    ASSERT_EQUAL(responses.size(), 1);
    const Node expected4 = Node(std::map<std::string, Node>{
            {"route_length", Node(int(2760000))},
            {"stop_count", Node(int(5))},
            {"unique_stop_count", Node(int(3))},
            {"curvature", Node(131.808412)},
            {"request_id", Node(int(1971))}
        });
    std::ostringstream os2, os3;
    os2 << responses[0];
    os3 << expected4;
    ASSERT_EQUAL(os2.str(), os3.str());
}

void TestAddStopDB() {
    TransportDatabase db;
    // Add one stop to db and check if it exists
    using namespace Json;
    auto node = Node(std::map<std::string, Node>{
        {"type", Node("Stop"s)},
        {"longitude", Node(37.20829)},
        {"name", Node("Tolstopaltsevo"s)},
        {"latitude", Node(55.611087)}
    });
    auto request_holder = ParseRequest(node, true);
    const auto& request = static_cast<const ModifyRequest&>(*request_holder);

    request.Process(db);

    Node stop_info = db.GetStopInfo("Tolstopaltsevo", 123);
    const Node expected = Node(std::map<std::string, Node>{
            {"buses", Node(std::vector<Node>{})},
            {"request_id", Node(123)}
        });
    ASSERT_EQUAL(expected, stop_info);
}

void TestAddBusRouteDB() {\
    // 1. Add one stop;
    // 2. Add Bus with route contains two stops including the first one
    // 3. Check both stops for coordinates
    // 4. Add Second stop
    // 5. Check both stops for coordinates
    // 6. Check random stop that doesn't exist
    // 7. Check bus route

    using namespace Json;
    TransportDatabase db;
    // 1. "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"
    auto request_holder = ParseRequest(Node(std::map<std::string, Node>{
        {"type", Node("Stop"s)},
        {"road_distances", Node(std::map<std::string, Node>{
            {"Marushkino", Node(3900)}
        })},
        {"longitude", Node(37.20829)},
        {"name", Node("Tolstopaltsevo"s)},
        {"latitude", Node(55.611087)}
    }), true);
    const auto& request = static_cast<const ModifyRequest&>(*request_holder);
    request.Process(db);

    // 2. "Bus 256: Tolstopaltsevo - Marushkino"
    request_holder = ParseRequest(Node(std::map<std::string, Node>{
        {"type", Node("Bus"s)},
        {"name", Node("256"s)},
        {"stops", Node(std::vector<Node>{
            Node("Tolstopaltsevo"s),
            Node("Marushkino"s)
        })},
        {"is_roundtrip", Node(false)}
    }), true);
    const auto& request2 = static_cast<const ModifyRequest&>(*request_holder);
    request2.Process(db);

    // 3. "Tolstopaltsevo" "Stop Tolstopaltsevo: buses 256";
    Node stop_info = db.GetStopInfo("Tolstopaltsevo", 123);
    const Node expected = Node(std::map<std::string, Node>{
            {"buses", Node(std::vector<Node>{"256"s})},
            {"request_id", Node(123)}
        });
    //
    ASSERT_EQUAL(expected, stop_info);

    // "Stop Marushkino: buses 256"
    stop_info = db.GetStopInfo("Marushkino", 321);
    const Node expected2 = Node(std::map<std::string, Node>{
            {"buses", Node(std::vector<Node>{"256"s})},
            {"request_id", Node(321)}
        });
    ASSERT_EQUAL(expected2, stop_info);

    // 4. "Stop Marushkino: 55.595884, 37.209755, 9900m to Tolstopaltsevo"
    request_holder = ParseRequest(Node(std::map<std::string, Node>{
        {"type", Node("Stop"s)},
        {"road_distances", Node(std::map<std::string, Node>{
            {"Tolstopaltsevo", Node(9900)}
        })},
        {"longitude", Node(37.209755)},
        {"name", Node("Marushkino"s)},
        {"latitude", Node(55.595884)}
    }), true);
    const auto& request3 = static_cast<const ModifyRequest&>(*request_holder);
    request3.Process(db);

    // 5.
    stop_info = db.GetStopInfo("Tolstopaltsevo", 123);
    ASSERT_EQUAL(expected, stop_info);

    // "Stop Marushkino: buses 256"
    stop_info = db.GetStopInfo("Marushkino", 321);
    ASSERT_EQUAL(expected2, stop_info);

    // 6. "Stop Rasskazovka: not found"
    stop_info = db.GetStopInfo("Rasskazovka", 322);
    const Node expected4 = Node(std::map<std::string, Node>{
            {"error_message", Node(std::string("not found"))},
            {"request_id", Node(322)}
        });
    ASSERT_EQUAL(expected4, stop_info);

    // 7. "Bus 256: 3 stops on route, 2 unique stops, 13800 route length, 4.075607 curvature"
    Node bus_info = db.GetBusInfo("256", 911);
    const Node expected5 = Node(std::map<std::string, Node>{
            {"route_length", Node(13800)},
            {"stop_count", Node(3)},
            {"unique_stop_count", Node(2)},
            {"curvature", Node(4.075607)},
            {"request_id", Node(911)}
        });
    std::ostringstream os1, os2;
    os1 << bus_info;
    os2 << expected5;
    ASSERT_EQUAL(os1.str(), os2.str());
}

void TestStopParseRequest() {
    // "Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"
    using namespace Json;
    Node input = Node(std::map<std::string, Node>{
        {"type", Node("Stop"s)},
        {"road_distances", Node(std::map<std::string, Node>{
            {"Marushkino", Node(3900)}
        })},
        {"longitude", Node(37.20829)},
        {"name", Node("Tolstopaltsevo"s)},
        {"latitude", Node(55.611087)}
    });
    auto request = std::make_unique<AddStopRequest>();
    request->ParseFrom(input);
    ASSERT_EQUAL(request->stop.GetName(), "Tolstopaltsevo");
    ASSERT_EQUAL(request->stop.GetLatitude().value(), 55.611087);
    ASSERT_EQUAL(request->stop.GetLongitude().value(), 37.20829);
    ASSERT_EQUAL(request->stop.GetDistances().size(), 1);

    const auto& [stop_name, distance] = *request->stop.GetDistances().begin();
    ASSERT_EQUAL(stop_name, "Marushkino");
    ASSERT_EQUAL(distance, 3900);
}

void TestMapOfStops() {
    TransportDatabase db;
    // Add one stop to db and check if it exists
    // "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"
    using namespace Json;
    auto request_holder = ParseRequest(Node(std::map<std::string, Node>{
        {"type", Node("Stop"s)},
        {"road_distances", Node(std::map<std::string, Node>{
            {"Marushkino", Node(3900)}
        })},
        {"longitude", Node(37.20829)},
        {"name", Node("Tolstopaltsevo"s)},
        {"latitude", Node(55.611087)}
    }), true);
    const auto& request = static_cast<const ModifyRequest&>(*request_holder);

    request.Process(db);

    // "Stop Tolstopaltsevo: no buses"
    Node stop_info = db.GetStopInfo("Tolstopaltsevo", 123);
    const Node expected = Node(std::map<std::string, Node>{
            {"buses", Node(std::vector<Node>{})},
            {"request_id", Node(123)}
        });
    ASSERT_EQUAL(expected, stop_info);

    // "Stop Marushkino: not found"
    stop_info = db.GetStopInfo("Marushkino", 321);
    const Node expected2 = Node(std::map<std::string, Node>{
            {"error_message", Node(std::string("not found"))},
            {"request_id", Node(321)}
        });
    ASSERT_EQUAL(expected2, stop_info);

    // Check if distances Tolstopaltsevo -> Marushkino is 3900
    auto distance = db.GetStop("Tolstopaltsevo")->GetDistance("Marushkino");
    ASSERT_EQUAL(*distance, 3900);

    // Add bus route
    // "Bus 256: Tolstopaltsevo - Marushkino"
    request_holder = ParseRequest(Node(std::map<std::string, Node>{
        {"type", Node("Bus"s)},
        {"name", Node("256"s)},
        {"stops", Node(std::vector<Node>{
            Node("Tolstopaltsevo"s),
            Node("Marushkino"s)
        })},
        {"is_roundtrip", Node(false)}
    }), true);
    const auto& request2 = static_cast<const ModifyRequest&>(*request_holder);
    request2.Process(db);

    // "Stop Marushkino: 55.595884, 37.209755"
    request_holder = ParseRequest(Node(std::map<std::string, Node>{
        {"type", Node("Stop"s)},
        {"longitude", Node(37.209755)},
        {"name", Node("Marushkino"s)},
        {"latitude", Node(55.595884)}
    }), true);
    const auto& request3 = static_cast<const ModifyRequest&>(*request_holder);
    request3.Process(db);

    // Check bus route
    // "Bus 256: 3 stops on route, 2 unique stops, 7800 route length, 2.303604 curvature"
    Node bus_info = db.GetBusInfo("256", 234);
    const Node expected3 = Node(std::map<std::string, Node>{
            {"route_length", Node(7800)},
            {"stop_count", Node(3)},
            {"unique_stop_count", Node(2)},
            {"curvature", Node(2.303604)},
            {"request_id", Node(234)}
        });
    std::ostringstream os1, os2;
    os1 << bus_info;
    os2 << expected3;
    ASSERT_EQUAL(os1.str(), os2.str());
}

void TestAllTransportDB() {
    TestRunner tr;
    RUN_TEST(tr, TestMapOfStops);
    // RUN_TEST(tr, TestStopParseRequest);
    // RUN_TEST(tr, TestAddStopDB);
    // RUN_TEST(tr, TestAddBusRouteDB);
    // RUN_TEST(tr, TestGetBus);
}