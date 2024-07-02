#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <memory>
#include <set>

#include "string_parses.h"

const double PI = 3.1415926535;
const double EARTH_RADIUS = 6371000;

class Stop {
public:

    Stop() = default;
    Stop(std::string name) {
        stop_name = name;
    }

    static Stop ParseFrom(std::string_view input) {
        // input = "X: latitude, longitude, D1m to stop1, D2m to stop2, ..."
        std::string name = std::string(ReadToken(input, ": "));
        const double latitude = ConvertToDouble(ReadToken(input, ", "));
        const double longitude = ConvertToDouble(ReadToken(input, ", "));
        if(input.empty()) {
            return {name, latitude, longitude};
        } else {
            Stop stop(name, latitude, longitude);
            while (!input.empty()) {
                std::string_view token = ReadToken(input, ", ");
                double distance = ConvertToDouble(ReadToken(token, "m to "));
                std::string stop_name = std::string(token);
                stop.AddDistance(stop_name, distance);
            }
            return stop;
        }
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

    void AddBusToStop(const std::string& bus_number) {
        buses_for_stop.insert(bus_number);
    }

    const std::set<std::string>& GetBuses() const {
        return buses_for_stop;
    }

    void AddDistance(const std::string& stop_name, double distance) {
        distances[stop_name] = distance;
    }

    void AddDistances(const std::unordered_map<std::string, double>& dist) {
        distances = std::move(dist);
    }

    double GetDistance(const std::string& stop_name) const {
        return distances.at(stop_name);
    }

    const std::unordered_map<std::string, double>& GetDistances() const {
        return distances;
    }

private:
    std::string stop_name;
    std::optional<double> latitude;
    std::optional<double> longitude;
    std::set<std::string> buses_for_stop;
    std::unordered_map<std::string, double> distances;

    Stop(std::string name, double latitude, double longitude) 
        : stop_name(name), latitude(latitude), longitude(longitude) {}
};

class Bus {
public:
    Bus() = default;
    Bus(std::string number, std::vector<std::string> stops) 
        : bus_number(number) {
            for (auto& stop : stops)
                route.push_back(std::move(stop));
        }

    std::vector<std::string> GetRoute() const {
        return route;
    }

private:
    std::string bus_number;
    std::vector<std::string> route;
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
            auto stop_distances = stop.GetDistances();
            if (!stop_distances.empty()) {
                for (const auto& [stop_name, distance] : stop_distances) {
                    // Add distance to stop forward and backward
                    it_stop->second.AddDistance(stop_name, distance);
                    map_of_stops[stop_name].AddDistance(it_stop->first, distance);
                }
            }
        }
    }

    void AddBus(const std::string& bus_number, const std::vector<std::string>& stops_on_route) {
        for (const auto& stop : stops_on_route) {
            AddStop(stop);
            map_of_stops[stop].AddBusToStop(bus_number);
        }
        buses[bus_number] = Bus(move(bus_number), move(stops_on_route));
    }

    std::string GetStopInfo(const std::string& stop_name) const {
        std::stringstream res;
        res << std::fixed << std::setprecision(6) << "Stop " << stop_name << ": ";
        auto it = map_of_stops.find(stop_name);
        if (it != map_of_stops.end()) {
            auto buses = it->second.GetBuses();
            if (buses.size() == 0) {
                res << "no buses";
            } else {
                res << "buses";
                for (const auto& bus : buses) {
                    res << " " << bus;
                }
            }
        } else {
            res << "not found";
        }
        return res.str();
    }

    std::string GetBusInfo(const std::string& bus_number) const {
        // output = "Bus X: R stops on route, U unique stops, L route length"
        std::stringstream ss;
        ss << "Bus " << bus_number << ": ";

        if (buses.count(bus_number) == 0) {
            ss << "not found";
        } else {
            auto route = buses.at(bus_number).GetRoute();
            ss << route.size() << " stops on route, ";
            std::unordered_set<std::string> unique_stops(route.begin(), route.end());
            ss << unique_stops.size() << " unique stops, ";
            double compute_route_length = 0.;
            double fact_route_length = 0.;
            for (size_t i = 1; i < route.size(); ++i) {
                compute_route_length += ComputeDistance(map_of_stops.at(route[i - 1]), map_of_stops.at(route[i]));
                //fact_route_length += map_of_stops.at(route[i - 1]).GetDistance(route[i]);
            }
            ss << fact_route_length << " route length, ";
            // curvature
            double c = fact_route_length / compute_route_length;
            ss << c << " curvature";
        }
        return ss.str();
    }

    Bus GetBus(const std::string& bus_number) const {
        return buses.at(bus_number);
    }

    static double ComputeDistance(const Stop& lhs, const Stop& rhs) {
        if (!lhs.GetLatitude() || !lhs.GetLongitude() || !rhs.GetLatitude() || !rhs.GetLongitude()) {
            std::cerr << "NO coordinates" << std::endl;
        }

        double lhs_lat = toRadians(*lhs.GetLatitude());
        double rhs_lat = toRadians(*rhs.GetLatitude());
        double lhs_lon = toRadians(*lhs.GetLongitude());
        double rhs_lon = toRadians(*rhs.GetLongitude());

        return acos(sin(lhs_lat) * sin(rhs_lat) + 
                    cos(lhs_lat) * cos(rhs_lat) *
                    cos(fabs(lhs_lon - rhs_lon))) * EARTH_RADIUS;
    }

private:
    std::unordered_map<std::string, Bus> buses;
    std::unordered_map<std::string, Stop> map_of_stops;

    static double toRadians(double degree) {
        return degree * PI / 180.0;
    }
};

std::vector<std::string> ParseStops(std::string_view input) {
    // 1. stop1 - stop2 - ... - stopN: автобус следует от stop1 до stopN и обратно с указанными промежуточными остановками.
    // 2. stop1 > stop2 > ... > stopN > stop1: кольцевой маршрут с конечной stop1.
    // input = "256: stop1 - stop2 - ... - stopN"

    bool is_ring = input.find(" > ") != std::string::npos;
    std::vector<std::string> stops;
    
    std::string_view delimiter = is_ring ? " > " : " - ";

    while (!input.empty()) {
        stops.push_back(std::string(ReadToken(input, delimiter)));
    }

    // if !is_ring then add stops in reverse order
    if (!is_ring) {
        for (size_t i = stops.size() - 1; i > 0; --i) {
            stops.push_back(stops[i - 1]);
        }
    }

    return stops;
}