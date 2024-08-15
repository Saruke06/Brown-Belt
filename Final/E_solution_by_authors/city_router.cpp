#include "city_router.h"

using namespace std::string_literals;

void CityRouter::AddStop(Json::Dict const& dict)
{
    auto stop = ParseStopFromJson(dict);
    auto [it, YES] = all_stops.try_emplace(stop->st_name, stop);
    if (!YES) {
        it->second->point = stop->point;
        it->second->neighbors.swap(stop->neighbors);
    } else {
        it->second->index = all_stops.size() - 1;
    }
}

void CityRouter::AddBus(Json::Dict const& dict)
{
    auto bus = ParseBusFromJson(dict);
    auto& names_st = bus->empty_stops;
    for (auto const& st : names_st) {
        auto it = all_stops.find(st);
        if (it != all_stops.end()) {
            it->second->buses.insert(bus->name_bs);
            bus->stops.push_back(it->second);
        } else {
            auto curr_stop = std::make_shared<Stop>();
            curr_stop->st_name = st;
            curr_stop->buses.insert(bus->name_bs);
            auto [pos, _] = all_stops.emplace(st, curr_stop);
            pos->second->index = all_stops.size() - 1;
            bus->stops.push_back(curr_stop);
        }
    }
    all_buses.emplace(bus->name_bs, bus);
}

auto CityRouter::GetInfoForBus(std::string const& name) -> std::optional<BusInfo>
{
    if (auto it = all_buses.find(name); it != all_buses.end()) {
        BusInfo info;
        auto const& bus = it->second;
        info.cnt_stop = bus->empty_stops.size();
        info.cnt_uniq_stop = UniqueStopOnRoute(bus);
        info.linear = LinearRouteLength(bus);
        info.length = RouteLength(bus);
        info.curvature = 1.0 * info.linear / info.length;
        return info;
    }
    return std::nullopt;
}

auto CityRouter::GetInfoForStop(const Json::Dict& dict) -> Json::Dict
{
    Json::Dict request_dict;
    request_dict["request_id"] = dict.at("id").AsInt();
    auto& stopname = dict.at("name").AsString();
    auto stop = all_stops.find(stopname);
    if (stop != all_stops.end()) {
        std::vector<Json::Node> bus_json;
        auto const& set_bus = stop->second->buses;
        for (auto& bus : set_bus) {
            bus_json.emplace_back(bus);
        }
        request_dict["buses"] = Json::Node(std::move(bus_json));
    } else {
        request_dict["error_message"] = Json::Node("not found"s);
    }

    return request_dict;
}

auto CityRouter::GetInfoForBus(const Json::Dict& dict) -> Json::Dict
{
    Json::Dict request_dict;
    request_dict["request_id"] = dict.at("id").AsInt();
    auto const& busname = dict.at("name").AsString();
    auto bus = GetInfoForBus(busname);
    if (bus.has_value()) {
        request_dict["stop_count"] = Json::Node(bus->cnt_stop);
        request_dict["unique_stop_count"] = Json::Node(bus->cnt_uniq_stop);
        request_dict["route_length"] = Json::Node(bus->linear);
        request_dict["curvature"] = Json::Node(bus->curvature);
    } else {
        request_dict["error_message"] = Json::Node("not found"s);
    }

    return request_dict;
}
