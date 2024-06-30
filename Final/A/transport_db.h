#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <memory>

#include "string_parses.h"

#define prent(x) (std::cerr << #x << " = " << x << '\n')

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
        : bus_number(number) {
            for (auto& stop : stops)
                route.push_back(std::move(stop));
        }

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
        for (const auto& stop : stops_on_route) {
            AddStop(stop);
        }
        buses[bus_number] = Bus(move(bus_number), move(stops_on_route));
    }

    std::string GetStop(const std::string& stop_name) const {
        std::stringstream res;
        res << std::fixed << std::setprecision(6);
        auto it = map_of_stops.find(stop_name);
        if (it != map_of_stops.end()) {
            res << "Stop " << stop_name << " exists with latitude ";
            if (it->second.GetLatitude().has_value()) {
                res << *(it->second.GetLatitude());
            } else {
                res << "unknown";
            }
            res << " and longitude ";
            if (it->second.GetLongitude().has_value()) {
                res << *(it->second.GetLongitude());
            } else {
                res << "unknown";
            }
        } else {
            res << "NO stop " << stop_name << " in the DB";
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
            std::unordered_set<std::string> unique_stops;
            for (const auto& stop : route) {
                unique_stops.insert(stop.GetName());
            }
            ss << unique_stops.size() << " unique stops, ";
            double route_length = 0.;
            for (size_t i = 1; i < route.size(); ++i) {
                std::string stop1_name = route[i - 1].GetName();
                std::string stop2_name = route[i].GetName();
                route_length += ComputeDistance(map_of_stops.at(stop1_name), map_of_stops.at(stop2_name));
            }
            ss << route_length << " route length";
        }
        return ss.str();
    }

    static double ComputeDistance(const Stop& lhs, const Stop& rhs) {
        if (!lhs.GetLatitude() || !lhs.GetLongitude() || !rhs.GetLatitude() || !rhs.GetLongitude()) {
            std::cerr << "NO coordinates" << std::endl;
        }

        double lhs_lat = *lhs.GetLatitude() * PI / 180.0;
        double rhs_lat = *rhs.GetLatitude() * PI / 180.0;
        double lhs_lon = *lhs.GetLongitude() * PI / 180.0;
        double rhs_lon = *rhs.GetLongitude() * PI / 180.0;

        // prent(lhs_lat);
        // prent(sin(lhs_lat));
        // prent(cos(lhs_lat));
        // prent(rhs_lat);
        // prent(sin(rhs_lat));
        // prent(cos(rhs_lat));
        // prent(lhs_lon);
        // prent(sin(lhs_lon));
        // prent(cos(lhs_lon));
        // prent(rhs_lon);
        // prent(sin(rhs_lon));
        // prent(cos(rhs_lon));

        // double SinSin = sin(lhs_lat) * sin(rhs_lat);
        // double CosCos = cos(lhs_lat) * cos(rhs_lat);
        // double Cos = cos(abs(lhs_lon - rhs_lon));
        // double CosCosCos = CosCos * Cos;
        // double Acos = acos(SinSin + CosCosCos);

        // prent(SinSin);
        // prent(CosCos);
        // prent(Cos);
        // prent(CosCosCos);
        // prent(Acos);

        // double should = Acos * EARTH_RADIUS;

        // double ret = acos(sin(lhs_lat) * sin(rhs_lat) + 
        //             cos(lhs_lat) * cos(rhs_lat) *
        //             cos(abs(lhs_lon - rhs_lon))) * EARTH_RADIUS;

        // prent(should);
        // prent(ret);

        return acos(sin(lhs_lat) * sin(rhs_lat) + 
                    cos(lhs_lat) * cos(rhs_lat) *
                    cos(abs(lhs_lon - rhs_lon))) * EARTH_RADIUS;
    }

private:
    std::unordered_map<std::string, Bus> buses;
    std::unordered_map<std::string, Stop> map_of_stops;
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