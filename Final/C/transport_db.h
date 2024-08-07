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
#include <optional>

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
                int distance = ConvertToInt<int>(ReadToken(token, "m to "));
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

    void AddDistance(const std::string& stop_name, int distance) {
        distances[stop_name] = distance;
    }

    void AddDistances(const std::unordered_map<std::string, int>& dist) {
        distances = std::move(dist);
    }

    std::optional<int> GetDistance(const std::string& stop_name) const {
        if (distances.count(stop_name) == 0) {
            return std::nullopt;
        }
        return distances.at(stop_name);
    }

    const auto& GetDistances() const {
        return distances;
    }

private:
    std::string stop_name;
    std::optional<double> latitude;
    std::optional<double> longitude;
    std::set<std::string> buses_for_stop;
    std::unordered_map<std::string, int> distances;

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

    void CheckAndSetStopCoordinates(const Stop& stop) {
        auto it_stop = stops.find(stop.GetName());
        bool has_coordinates = it_stop->second->GetLatitude().has_value() && it_stop->second->GetLongitude().has_value();
        if (!has_coordinates) {
            if (stop.GetLatitude().has_value() && stop.GetLongitude().has_value()) {
                it_stop->second->SetLatitude(stop.GetLatitude().value());
                it_stop->second->SetLongitude(stop.GetLongitude().value());
            }
        }
    }

    void AddReverseDistances(const Stop& stop) {
        for (const auto& [stop_name, distance] : stop.GetDistances()) {
            auto it = stops.find(stop_name);
            if (it != stops.end()) {
                it->second->AddDistance(stop.GetName(), distance);
            } else {
                stops[stop_name] = std::shared_ptr<Stop>(new Stop(stop_name));
                stops[stop_name]->AddDistance(stop.GetName(), distance);
            }
        }
    }

    void AddStop(const Stop& stop) {
        auto current_stop_name = stop.GetName();
        auto it_stop = stops.find(current_stop_name);
        if (it_stop == stops.end()) {
            stops[current_stop_name] = std::shared_ptr<Stop>(new Stop(stop));
            //AddReverseDistances(stop);

        } else {
            CheckAndSetStopCoordinates(stop);

            //AddReverseDistances(stop);

            // Merge stop.GetDistances() to stops[current_stop_name] without overwriting
            for (const auto& [stop_name, distance] : stop.GetDistances()) {
                if (it_stop->second->GetDistances().count(stop_name) == 0) {
                    it_stop->second->AddDistance(stop_name, distance);
                }
            }
        }
    }

    void AddBus(const std::string& bus_number, const std::vector<std::string>& stops_on_route) {
        for (const auto& stop : stops_on_route) {
            AddStop(stop);
            stops[stop]->AddBusToStop(bus_number);
        }
        buses[bus_number] = std::shared_ptr<Bus>(new Bus(move(bus_number), move(stops_on_route)));
    }

    std::string GetStopInfo(const std::string& stop_name) const {
        std::stringstream res;
        res << "Stop " << stop_name << ": ";
        auto it = stops.find(stop_name);
        if (it != stops.end()) {
            auto buses = it->second->GetBuses();
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
            auto route = buses.at(bus_number)->GetRoute();
            ss << route.size() << " stops on route, ";
            std::unordered_set<std::string> unique_stops(route.begin(), route.end());
            ss << unique_stops.size() << " unique stops, ";

            auto [route_length, fact_route_length] = ComputeRouteAndFactRouteLength(bus_number);

            ss << fact_route_length << " route length, " << std::fixed;
            // curvature
            double c = fact_route_length / route_length;
            ss << std::setprecision(6) << c << " curvature";
        }
        return ss.str();
    }

    const auto& GetBus(const std::string& bus_number) const {
        return buses.at(bus_number);
    }

    const auto& GetStop(const std::string& stop_name) const {
        return stops.at(stop_name);
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

    std::pair<double, int> ComputeRouteAndFactRouteLength(const std::string& bus_number) const {
        double route_length = 0.;
        int fact_route_length = 0.;
        const auto& route = buses.at(bus_number)->GetRoute();
        for (size_t i = 1; i < route.size(); ++i) {
            route_length += ComputeDistance(*stops.at(route[i - 1]), *stops.at(route[i]));
            fact_route_length += FindDistance(route[i - 1], route[i]);
        }
        return {route_length, fact_route_length};
    }

    int FindDistance(const std::string& stop1, const std::string& stop2) const {
        // если для stop1 нет расстояния до stop2, то вернуть расстояние от stop2 до stop1
        auto it = stops.find(stop1);
        if (it != stops.end()) {
            auto distance = it->second->GetDistance(stop2);
            if (distance.has_value()) {
                return distance.value();
            }
        }
        return stops.at(stop2)->GetDistance(stop1).value();
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Bus>> buses;
    std::unordered_map<std::string, std::shared_ptr<Stop>> stops;

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