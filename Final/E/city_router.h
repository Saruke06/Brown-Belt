#pragma once
#include "Types.h"

class CityRouter {
    public:
        std::unordered_map<std::string, ptrStop> all_stops;
        std::unordered_map<std::string, ptrBus> all_buses;
    public:
        CityRouter() = default;
        void AddStop(const Json::Dict& dict);
        void AddBus(const Json::Dict& dict);

        std::optional<BusInfo> GetInfoForBus(const std::string& name) const;
        Json::Dict GetInfoForStop(const Json::Dict& dict) const;
        Json::Dict GetInfoForBus(const Json::Dict& dict) const;
};