#pragma once
#include "Types.h"

class CityRouter {
    public:
        std::unordered_map<std::string, ptrStop> all_stops;
        std::unordered_map<std::string, ptrBus> all_buses;
    public:
        CityRouter() = default;
        void AddStop(Json::Dict const& dict);
        void AddBus(Json::Dict const& dict);

        std::optional<BusInfo> GetInfoForBus(std::string const& name) const;
        Json::Dict GetInfoForStop(const Json::Dict& dict) const;
        Json::Dict GetInfoForBus(const Json::Dict& dict) const;
};