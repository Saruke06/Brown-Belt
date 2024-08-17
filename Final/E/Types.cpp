#include "Types.h"
#include <cmath>

inline static double const DEGREE = 180.0;

using namespace std::string_literals;
Coordinate::Coordinate(double lat, double lon) 
    : latitude(lat * acos(-1) / DEGREE)
    , longitude(lon * acos(-1) / DEGREE)
{
}

auto LinearDistanse(ptrStop lhs, ptrStop rhs) -> int
{
    int dist = 0;
    if (!lhs->neighbors.empty() || !rhs->neighbors.empty()) {
        auto it = lhs->neighbors.find(rhs->stop_name);
        if (it != lhs->neighbors.end()) {
            dist += it->second;
        } else {
            dist += rhs->neighbors.at(lhs->stop_name);
        }
    }
    return dist;
}

auto GeoDistance(ptrStop lhs, ptrStop rhs) -> double
{
    const double RADIUS_EARTH = 6'371'000;
    return acos(
               sin(lhs->point.latitude) * sin(rhs->point.latitude) + 
               cos(lhs->point.latitude) * cos(rhs->point.latitude) * cos(std::abs(lhs->point.longitude - rhs->point.longitude)))
               * RADIUS_EARTH;
}

auto ParseStopFromJson(const Json::Dict& dict) -> ptrStop
{
    Stop stop;
    stop.stop_name = dict.at("name").AsString();

    stop.point = {
        dict.at("latitude").AsDouble(), dict.at("longitude").AsDouble()
    };
    if (dict.count("road_distances") != 0U) {
        for (const auto& neigh : dict.at("road_distances").AsMap()) {
            stop.neighbors.emplace(neigh.first, neigh.second.AsInt());
        }
    }
    return std::make_shared<Stop>(stop);
}

auto ParseBusFromJson(const Json::Dict& dict) -> ptrBus
{
    Bus bus;
    bus.bus_name = dict.at("name").AsString();
    for (auto& stop : dict.at("stops").AsArray()) {
        bus.empty_stops.push_back(stop.AsString());
    }
    bus.isTreap = dict.at("is_roundtrip").AsBool();

    if (!bus.isTreap) {
        for (int i = (int)bus.empty_stops.size() - 2; i >= 0; --i) {
            bus.empty_stops.push_back(bus.empty_stops[i]);
        }
    }

    return std::make_shared<Bus>(bus);
}

auto RouteLength(ptrBus bus) -> double
{
    double length = 0.0;
    for (size_t i = 1; i < bus->stops.size(); ++i) {
        length += GeoDistance(bus->stops[i - 1], bus->stops[i]);
    }
    return length;
}

auto LinearRouteLength(ptrBus bus) -> int
{
    int length = 0;
    const auto& stops = bus->stops;
    for (size_t i = 1; i < stops.size(); ++i) {
        length += LinearDistanse(stops[i - 1], stops[i]);
    }
    return length;
}

auto UniqueStopOnRoute(ptrBus bus) -> int
{
    return std::set<std::string>{bus->empty_stops.begin(), bus->empty_stops.end()}.size();
}

RoutingSetting::RoutingSetting(double wait_time, double velocity)
    : bus_wait_time(wait_time)
    , bus_velocity(velocity)
{
}

Item::Item(Graph::EDGE_TYPE edge_type, std::string name, Minutes time, size_t span)
    : edge_type(edge_type)
    , name(std::move(name))
    , time(time)
    , span(span)
{
}
