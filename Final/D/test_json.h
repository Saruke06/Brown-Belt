#pragma once

#include "json.h"
#include "../../test_runner.h"

void TestJsonParseStop() {
    std::istringstream is(
    R"({
        "road_distances": {
            "Marushkino": 3900
        },
        "type": "Stop",
        "latitude": 55.611087,
        "name": "Tolstopaltsevo",
        "longitude": 37.20829
    })");
    
    const auto stop = Json::Load(is).GetRoot().AsMap();
    
    ASSERT_EQUAL(stop.at("type").AsString(), "Stop");
    ASSERT_EQUAL(stop.at("name").AsString(), "Tolstopaltsevo");
    ASSERT_EQUAL(stop.at("latitude").AsDouble(), 55.611087);
    ASSERT_EQUAL(stop.at("longitude").AsDouble(), 37.20829);
}

// void TestJsonOutput() {
//     std::ostringstream os;
//     Json::Node node(std::map<std::string, Json::Node>{
//         {"type", Json::Node("Bus"s)},
//         {"name", Json::Node("750"s)}, 
//         {"stops", Json::Node(std::vector<Json::Node>{
//             Json::Node("Tolstopaltsevo"s), 
//             Json::Node("Marushkino"s)})}
//     });
//     os << node;
//     ASSERT_EQUAL(os.str(), 
// R"({
//   "name":   "750",
//   "stops":   [
//     "Tolstopaltsevo",
//     "Marushkino"
//   ],
//   "type":   "Bus"
// })");
// }

void TestDocument() {
    using namespace Json;
    try {
        Document doc = Load();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void TestAllJson() {
    TestRunner tr;
    RUN_TEST(tr, TestJsonParseStop);
    //RUN_TEST(tr, TestJsonOutput);
    RUN_TEST(tr, TestDocument);
}