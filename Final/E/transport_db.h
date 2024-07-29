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
#include <map>
#include <optional>

#include "string_parses.h"
#include "json.h"

const double PI = 3.1415926535;
const double EARTH_RADIUS = 6371000;

class Stop {
public:

    Stop() = default;
    Stop(std::string name) {
        stop_name = name;
    }

    static Stop ParseFrom(const Json::Node& request_node) {
        // input = "X: latitude, longitude, D1m to stop1, D2m to stop2, ..."
        std::string name = request_node.AsMap().at("name").AsString();
        const double latitude = request_node.AsMap().at("latitude").AsDouble();
        const double longitude = request_node.AsMap().at("longitude").AsDouble();
        if(request_node.AsMap().count("road_distances") == 0) {
            return {name, latitude, longitude};
        } else {
            Stop stop(name, latitude, longitude);
            for (const auto& [stop_name, distance] : request_node.AsMap().at("road_distances").AsMap()) {
                stop.AddDistance(stop_name, distance.AsInt());
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

    std::string GetStopInfo(const std::string& stop_name, int id) const {
        // Example: 
        /*      
            "buses": [
              "256",
              "828"
            ],
            "request_id": 1042838872
        */
        std::vector<std::string> buses;
        std::ostringstream os;
        auto it = stops.find(stop_name);
        if (it != stops.end()) {
            for (const auto& bus : it->second->GetBuses()) {
                buses.push_back(std::string(bus));
            }
            if (!buses.empty()) {
                os << "{\n";
                os << "  \"buses\": [\n";
                for (size_t i = 0; i < buses.size(); ++i) {
                    os << "    \"" << buses[i] << "\"";
                    if (i != buses.size() - 1) {
                        os << ",\n";
                    } else {
                        os << "\n";
                    }
                }
                os << "  ],\n";
                os << "  \"request_id\": " << id << "\n";
                os << "}";
            } else {
                os << "{\n";
                os << "  \"buses\": [],\n";
                os << "  \"request_id\": " << id << "\n";
                os << "}";
            }
        } else {
            os << "{\n";
            os << "  \"error_message\": \"not found\",\n";
            os << "  \"request_id\": " << id << "\n";
            os << "}";
        }
        return os.str();
    }

    std::string GetBusInfo(const std::string& bus_number, int id) const {
        // Rebuild as JSON
        // Example:
        /* 
            "route_length": 27600,
            "request_id": 519139350,
            "curvature": 1.31808,
            "stop_count": 5,
            "unique_stop_count": 3
        */
        
        std::ostringstream os;
        if (buses.count(bus_number) == 0) {
            os << "{\n";
            os << "  \"error_message\": \"not found\",\n";
            os << "  \"request_id\": " << id << "\n";
            os << "}";
            return os.str();
        } else {
            auto route = buses.at(bus_number)->GetRoute();
            std::unordered_set<std::string> unique_stops(route.begin(), route.end());
            auto [route_length, fact_route_length] = ComputeRouteAndFactRouteLength(bus_number);
            double c = fact_route_length / route_length;
            os << "{\n";
            os << "  \"route_length\": " << fact_route_length << ",\n";
            os << "  \"request_id\": " << id << ",\n";
            os << "  \"curvature\": " << std::fixed << std::setprecision(6) << c << ",\n";
            os << "  \"stop_count\": " << route.size() << ",\n";
            os << "  \"unique_stop_count\": " << unique_stops.size() << "\n";
            os << "}";
            return os.str();
        }
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
