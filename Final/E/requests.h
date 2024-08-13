#pragma once

#include <string>
#include <memory>
#include <variant>

#include "transport_db.h"
#include "json.h"


namespace Requests {
  struct Stop {
    std::string name;

    Json::Dict Process(const Database& db) const {
      const auto* stop = db.GetStop(name);
      Json::Dict dict;
      if (!stop) {
        dict["error_message"] = Json::Node(std::string("not found"));
      } else {
        std::vector<Json::Node> bus_nodes;
        bus_nodes.reserve(stop->bus_names.size());
        for (const auto& bus_name : stop->bus_names) {
          bus_nodes.emplace_back(bus_name);
        }
        dict["buses"] = Json::Node(std::move(bus_nodes));
      }
      return dict;
    }
  };

  struct Bus {
    std::string name;

    Json::Dict Process(const Database& db) const {
      const auto* bus = db.GetBus(name);
      Json::Dict dict;
      if (!bus) {
        dict["error_message"] = Json::Node(std::string("not found"));
      } else {
        dict = {
            {"stop_count", Json::Node(static_cast<int>(bus->stop_count))},
            {"unique_stop_count", Json::Node(static_cast<int>(bus->unique_stop_count))},
            {"route_length", Json::Node(bus->road_route_length)},
            {"curvature", Json::Node(bus->road_route_length / bus->geo_route_length)},
        };
      }
      return dict;
    }
  };

  struct Route {
    std::string from_name;
    std::string to_name;

    Json::Dict Process(const Database& db) const {
      // find route from from_name to to_name
      // {
      //   "total_time": <суммарное время>,
      //   "items": [
      //       <элементы маршрута>
      //   ]
      // }
      Json::Dict dict;

      return dict;
    }
  };

  std::variant<Stop, Bus, Route> ReadQuery(const Json::Dict& attrs) {
    if (attrs.at("type").AsString() == "Bus") {
      return Bus{attrs.at("name").AsString()};
    } else if (attrs.at("type").AsString() == "Stop") {
      return Stop{attrs.at("name").AsString()};
    } else if (attrs.at("type").AsString() == "Route") {
      return Route{
          .from_name = attrs.at("from").AsString(),
          .to_name = attrs.at("to").AsString(),
      };
    } else {
      throw std::invalid_argument("Unknown request type");
    }
  }
}

std::vector<Json::Node> ProcessStatRequests(const Database& transport_database, const std::vector<Json::Node>& stat_requests) {
  std::vector<Json::Node> responses;
  responses.reserve(stat_requests.size());
  for (const Json::Node& request_node : stat_requests) {
    Json::Dict dict = visit([&transport_database](const auto& request) {
                              return request.Process(transport_database);
                            },
                            Requests::ReadQuery(request_node.AsMap()));
    dict["request_id"] = Json::Node(request_node.AsMap().at("id").AsInt());
    responses.push_back(Json::Node(dict));
  }
  return responses;
}