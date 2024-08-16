#pragma once

#include "graph.h"
#include "json.h"
#include <set>
#include <unordered_map>
#include <memory>
#include <string>

struct Coordinate {
    double latitude;
    double longitude;
    Coordinate() = default;
    Coordinate(double lat, double lon);
};

struct Stop {
    std::string stop_name;
    Coordinate point {};
    Stop() = default;
    std::set<std::string> buses;
    std::unordered_map<std::string, int> neighbors;
};

using ptrStop = std::shared_ptr<Stop>;

struct Bus {
    std::string bus_name;
    std::vector<ptrStop> stops;
    std::vector<std::string> empty_stops;
    bool isTreap = false;
};

using ptrBus = std::shared_ptr<Bus>;

struct BusInfo {
    int cnt_stop = 0;
    int cnt_uniq_stop = 0;
    double length = 0.0;
    int linear = 0;
    double curvature = 0.0;
    BusInfo() = default;
};

struct RoutingSetting {
    RoutingSetting() = default;
    RoutingSetting(double, double);
    double bus_wait_time;
    double bus_velocity;
};

using Minutes = double;

struct Item {
    Item(Graph::EDGE_TYPE edge_type, std::string name, Minutes time, size_t span);
    Graph::EDGE_TYPE edge_type;
    std::string name;
    Minutes time = 0.0;
    size_t span = 0;
};

auto LinearDistance(ptrStop lhs, ptrStop rhs) -> int;

auto GeoDistance(ptrStop lhs, ptrStop rhs) -> double;

auto ParseStopFromJson(const Json::Dict& dict) -> ptrStop;

auto ParseBusFromJson(const Json::Dict& dict) -> ptrBus;

auto RouteLength(ptrBus bus) -> double;

auto LinearRouteLength(ptrBus bus) -> int;

auto UniqueStopOnRoute(ptrBus bus) -> int;