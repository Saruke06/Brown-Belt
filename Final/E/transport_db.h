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

#include "json.h"
#include "router.h"


namespace Sphere {
  const double PI = 3.1415926535;

  double ConvertDegreesToRadians(double degrees) {
    return degrees * PI / 180.0;
  }

  const double EARTH_RADIUS = 6'371'000;

  struct Point {
    double latitude;
    double longitude;

    static Point FromDegrees(double latitude, double longitude) {
      return {
        ConvertDegreesToRadians(latitude),
        ConvertDegreesToRadians(longitude)
      };
    }
  };

  double Distance(Point lhs, Point rhs) {
    lhs = Point::FromDegrees(lhs.latitude, lhs.longitude);
    rhs = Point::FromDegrees(rhs.latitude, rhs.longitude);
    return acos(
      sin(lhs.latitude) * sin(rhs.latitude)
      + cos(lhs.latitude) * cos(rhs.latitude) * cos(abs(lhs.longitude - rhs.longitude))
    ) * EARTH_RADIUS;
  }
}

namespace Descriptions {
  struct Stop {
    std::string name;
    Sphere::Point position;
    std::unordered_map<std::string, double> distances;

    static Stop ParseFrom(const Json::Dict& attrs) {
      Stop stop = {
          .name = attrs.at("name").AsString(),
          .position = {
              .latitude = attrs.at("latitude").AsDouble(),
              .longitude = attrs.at("longitude").AsDouble(),
          }
      };
      if (attrs.count("road_distances") > 0) {
        for (const auto& [neighbour_stop, distance_node] : attrs.at("road_distances").AsMap()) {
          stop.distances[neighbour_stop] = distance_node.AsInt();
        }
      }
      return stop;
    }
  };

  std::vector<std::string> ParseStops(const std::vector<Json::Node>& stop_nodes, bool is_roundtrip) {
    std::vector<std::string> stops;
    stops.reserve(stop_nodes.size());
    for (const Json::Node& stop_node : stop_nodes) {
      stops.push_back(stop_node.AsString());
    }
    if (is_roundtrip || stops.size() <= 1) {
      return stops;
    }
    stops.reserve(stops.size() * 2 - 1);  // end stop is not repeated
    for (size_t stop_idx = stops.size() - 1; stop_idx > 0; --stop_idx) {
      stops.push_back(stops[stop_idx - 1]);
    }
    return stops;
  }

  struct Bus {
    std::string name;
    std::vector<std::string> stops;

    static Bus ParseFrom(const Json::Dict& attrs) {
      return Bus{
          .name = attrs.at("name").AsString(),
          .stops = ParseStops(attrs.at("stops").AsArray(), attrs.at("is_roundtrip").AsBool()),
      };
    }
  };

  struct RouteSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
  };

  inline RouteSettings ParseRouteSettings(const Json::Dict& dict) {
    return {
        .bus_wait_time = dict.at("bus_wait_time").AsInt(),
        .bus_velocity = dict.at("bus_velocity").AsDouble(),
    };
  }

  using InputQuery = std::variant<Stop, Bus>;

  std::vector<InputQuery> ReadDescriptions(const std::vector<Json::Node>& nodes) {
    std::vector<InputQuery> result;
    result.reserve(nodes.size());

    for (const Json::Node& node : nodes) {
      const auto& node_dict = node.AsMap();
      if (node_dict.at("type").AsString() == "Bus") {
        result.push_back(Bus::ParseFrom(node_dict));
      } else {
        result.push_back(Stop::ParseFrom(node_dict));
      }
    }

    return result;
  }
}

namespace Responses {
  struct Stop {
    std::set<std::string> bus_names;
  };

  struct Bus {
    size_t stop_count = 0;
    size_t unique_stop_count = 0;
    double road_route_length = 0.0;
    double geo_route_length = 0.0;
  };
}

class Database {
public:
  using Bus = Responses::Bus;
  using Stop = Responses::Stop;

  explicit Database(std::vector<Descriptions::InputQuery> data, Descriptions::RouteSettings settings) : 
  routing_settings_(settings),
  graph_(0)
  {
    auto stops_end = std::partition(begin(data), end(data), [](const auto& item) {
      return std::holds_alternative<Descriptions::Stop>(item);
    });

    std::unordered_map<std::string, const Descriptions::Stop*> stop_info;
    for (const auto& item : Range{begin(data), stops_end}) {
      const auto& stop = std::get<Descriptions::Stop>(item);
      stop_info[stop.name] = &stop;
      stops.insert({stop.name, {}});
    }

    // resize graph.size() to the number of stops * 2
    graph_ = Graph::DirectedWeightedGraph<double>(stops.size() * 2);

    // for each Stop, there are two vertices in the graph: 
    // one to enter the stop and one to exit the stop after bus_wait_time
    for (const auto& stop : Range{begin(data), stops_end}) {
      graph_.AddEdge({stops.size() * 2 - 2, stops.size() * 2 - 1, routing_settings_.bus_wait_time});
      graph_.AddEdge({stops.size() * 2 - 1, stops.size() * 2 - 2, routing_settings_.bus_wait_time});
    }

    for (const auto& item : Range{stops_end, end(data)}) {
      const auto& bus = std::get<Descriptions::Bus>(item);

      buses[bus.name] = Bus{
        bus.stops.size(),
        ComputeUniqueItemsCount(AsRange(bus.stops)),
        ComputeRoadRouteLength(bus.stops, stop_info),
        ComputeGeoRouteDistance(bus.stops, stop_info)
      };

      for (const std::string& stop_name : bus.stops) {
        stops.at(stop_name).bus_names.insert(bus.name);
      }
    }


    router_ = std::make_unique<Graph::Router<double>>(graph_);
  }

  const Stop* GetStop(const std::string& name) const {
    return GetValuePointer(stops, name);
  }

  const Bus* GetBus(const std::string& name) const {
    return GetValuePointer(buses, name);
  }

private:
  static double ComputeRoadRouteLength(
    const std::vector<std::string>& stops,
    const std::unordered_map<std::string, const Descriptions::Stop*> stop_info
  ) {
    double result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
      result += Distance(*stop_info.at(stops[i - 1]), *stop_info.at(stops[i]));
    }
    return result;
  }

  static double ComputeGeoRouteDistance(
    const std::vector<std::string>& stops,
    const std::unordered_map<std::string, const Descriptions::Stop*> stop_info
  ) {
    double result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
      result += Sphere::Distance(
        stop_info.at(stops[i - 1])->position, stop_info.at(stops[i])->position
      );
    }
    return result;
  }

  static double Distance(const Descriptions::Stop& lhs, const Descriptions::Stop& rhs) {
    if (auto it = lhs.distances.find(rhs.name); it != lhs.distances.end()) {
      return it->second;
    } else if (auto it2 = rhs.distances.find(lhs.name); it2 != rhs.distances.end()) {
      return it2->second;
    } else {
      return Sphere::Distance(lhs.position, rhs.position);
    }
  }

  std::unordered_map<std::string, Stop> stops;
  std::unordered_map<std::string, Bus> buses;
  // for Route
  Descriptions::RouteSettings routing_settings_;
  Graph::DirectedWeightedGraph<double> graph_;
  std::unique_ptr<Graph::Router<double>> router_;
};