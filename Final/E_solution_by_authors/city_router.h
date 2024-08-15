#pragma once
#include "Types.h"
#include <optional>

class CityRouter {
public:
    std::unordered_map<std::string, ptrStop> all_stops;
    std::unordered_map<std::string, ptrBus> all_buses;

public:
    CityRouter() = default;
    void AddStop(Json::Dict const& dict);
    void AddBus(Json::Dict const& dict);

    auto GetInfoForBus(std::string const& name) -> std::optional<BusInfo>;
    auto GetInfoForStop(const Json::Dict& dict) -> Json::Dict;
    auto GetInfoForBus(const Json::Dict& dict) -> Json::Dict;
};
