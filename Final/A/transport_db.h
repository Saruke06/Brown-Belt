#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

#include "string_parses.h"

const double PI = 3.1415926535;
const double EARTH_RADIUS = 6'371'000;

class Stop {
public:

    Stop() = default;
    Stop(std::string name) {
        stop_name = name;
    }

    static Stop ParseFrom(std::string_view input) {
        std::string name = std::string(ReadToken(input, ": "));
        const double latitude = ConvertToDouble(ReadToken(input, ", "));
        const double longitude = ConvertToDouble(input);
        return {name, latitude, longitude};
    }

    void SetLatitude(double lat) {
        latitude = lat;
    }

    void SetLongitude(double lon) {
        longitude = lon;
    }

    std::string GetName() const {
        return stop_name;
    }

    std::optional<double> GetLatitude() const {
        return latitude;
    }

    std::optional<double> GetLongitude() const {
        return longitude;
    }

private:
    std::string stop_name;
    std::optional<double> latitude;
    std::optional<double> longitude;

    Stop(std::string name, double latitude, double longitude) 
        : stop_name(name), latitude(latitude), longitude(longitude) {}
};

class Bus {
public:
    Bus() = default;
    Bus(std::string number, std::vector<Stop> stops) 
        : bus_number(number), route(stops) {}

    std::vector<Stop> GetRoute() const {
        return route;
    }

private:
    std::string bus_number;
    std::vector<Stop> route;
};

class TransportDatabase {
public:
    TransportDatabase() = default;

    void AddStop(const Stop& stop) {
        auto stop_name = stop.GetName();
        auto it_stop = map_of_stops.find(stop_name);
        if (it_stop == map_of_stops.end()) {
            map_of_stops[stop_name] = std::move(stop);
        } else {
            bool has_coordinates = it_stop->second.GetLatitude().has_value() && it_stop->second.GetLongitude().has_value();
            if (!has_coordinates) {
                if (stop.GetLatitude().has_value() && stop.GetLongitude().has_value()) {
                    it_stop->second.SetLatitude(stop.GetLatitude().value());
                    it_stop->second.SetLongitude(stop.GetLongitude().value());
                }
            }
        }
    }

    void AddBus(const std::string& bus_number, const std::vector<Stop>& stops_on_route) {
        buses[bus_number] = Bus(bus_number, stops_on_route);
        for (const auto& stop : stops_on_route) {
            AddStop(stop);
        }
    }

    std::string GetBusInfo(const std::string& bus_number) const {
        // output = "Bus X: R stops on route, U unique stops, L route length"
        std::stringstream ss;
        ss.precision(6);
        ss << "Bus " << bus_number << ": ";

        if (buses.count(bus_number) == 0) {
            ss << "not found";
        } else {
            auto route = buses.at(bus_number).GetRoute();
            ss << route.size() << " stops on route, ";
            std::unordered_set<std::string> unique_stops;
            for (const auto& stop : route) {
                unique_stops.insert(stop.GetName());
            }
            ss << unique_stops.size() << " unique stops, ";
            double route_length = 0;
            for (size_t i = 1; i < route.size(); ++i) {
                // route_length += ComputeDistance(route[i - 1], route[i]);
            }
            ss << route_length << " route length";
        }
        return ss.str();
    }

private:
    std::unordered_map<std::string, Bus> buses;
    std::unordered_map<std::string, Stop> map_of_stops;

    double ComputeDistance(const Stop& lhs, const Stop& rhs) const {
        double lhs_lat = *lhs.GetLatitude() * PI / 180;
        double rhs_lat = *rhs.GetLatitude() * PI / 180;
        double lhs_lon = *lhs.GetLongitude() * PI / 180;
        double rhs_lon = *rhs.GetLongitude() * PI / 180;

        return acos(sin(lhs_lat) * sin(rhs_lat) + 
                    cos(lhs_lat) * cos(rhs_lat) *
                    cos(abs(lhs_lon - rhs_lon)) ) * EARTH_RADIUS;
    }
};

std::vector<Stop> ParseStops(std::string_view input) {
    // 1. stop1 - stop2 - ... - stopN: автобус следует от stop1 до stopN и обратно с указанными промежуточными остановками.
    // 2. stop1 > stop2 > ... > stopN > stop1: кольцевой маршрут с конечной stop1.
    // input = "256: stop1 - stop2 - ... - stopN"

    bool is_ring = input.find(" > ") != std::string::npos;
    std::vector<Stop> stops;
    
    std::string_view delimiter = is_ring ? " > " : " - ";

    while (!input.empty()) {
        stops.push_back(Stop(std::string(ReadToken(input, delimiter))));
    }

    // if !is_ring then add stops in reverse order
    if (!is_ring) {
        for (size_t i = stops.size() - 1; i > 0; --i) {
            stops.push_back(stops[i - 1]);
        }
    }

    return stops;
}