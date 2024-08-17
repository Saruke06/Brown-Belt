#include "city_router.h"

using namespace std::string_literals;

void CityRouter::AddStop(const Json::Dict& dict) {
    auto stop = ParseStopFromJson(dict);
    auto [it, YES] = all_stops.try_emplace(stop->stop_name, stop);
    if (!YES) {
        it->second->point = stop->point;
        it->second->neighbors.swap(stop->neighbors);
    }
}

void CityRouter::AddBus(const Json::Dict& dict) {
    auto bus = ParseBusFromJson(dict);
    auto& names_st = bus->empty_stops;
    for (const auto& st : names_st) {
        auto it = all_stops.find(st);
        if (it != all_stops.end()) {
            it->second->buses.insert(bus->bus_name);
            bus->stops.push_back(it->second);
        } else {
            auto curr_stop = std::make_shared<Stop>();
            curr_stop->stop_name = st;
            curr_stop->buses.insert(bus->bus_name);
            auto [pos, _] = all_stops.emplace(st, curr_stop);
            bus->stops.push_back(curr_stop);
        }
    }
    all_buses.emplace(bus->bus_name, bus);
}

Json::Dict CityRouter::GetInfoForStop(const Json::Dict& dict) const {
    Json::Dict request_dict;
    request_dict["request_id"] = dict.at("id").AsInt();
    auto& stop_name = dict.at("name").AsString();
    auto stop = all_stops.find(stop_name);
    if (stop != all_stops.end()) {
        std::vector<Json::Node> bus_json;
        const auto& set_bus = stop->second->buses;
        for (const auto& bus : set_bus) {
            bus_json.push_back(Json::Node(bus));
        }
        request_dict["buses"] = Json::Node(std::move(bus_json));
    } else {
        request_dict["error_message"] = Json::Node("not found"s);
    }

    return request_dict;
}

std::optional<BusInfo> CityRouter::GetInfoForBus(const std::string& name) const {
    if (auto it = all_buses.find(name); it != all_buses.end()) {
        BusInfo info;
        const auto& bus = it->second;
        info.cnt_stop = bus->empty_stops.size();
        info.cnt_uniq_stop = UniqueStopOnRoute(bus);
        info.linear = LinearRouteLength(bus);
        info.length = RouteLength(bus);
        info.curvature = 1.0 * info.linear / info.length;
        return info;
    }
    return std::nullopt;
}

Json::Dict CityRouter::GetInfoForBus(const Json::Dict& dict) const {
    Json::Dict request_dict;
    request_dict["request_id"] = dict.at("id").AsInt();
    auto& busname = dict.at("name").AsString();
    auto bus = GetInfoForBus(busname);
    if (bus.has_value()) {
        request_dict["stop_count"] = Json::Node(bus->cnt_stop);
        request_dict["unique_stop_count"] = Json::Node(bus->cnt_uniq_stop);
        request_dict["route_length"] = Json::Node(bus->length);
        request_dict["curvature"] = Json::Node(bus->curvature);
    } else {
        request_dict["error_message"] = Json::Node("not_found"s);
    }

    return request_dict;
}