#pragma once
#include "city_router.h"

class Builder {
    RoutingSetting route_setting {};
    std::unique_ptr<CityRouter> city_router = std::make_unique<CityRouter>();
    std::vector<Json::Node> requests;

public:
    Builder() = default;
    void ParseData(std::istream& input = std::cin);
    void PrintRequest(std::ostream& output = std::cout);
private:
    void BuildCityRouter(const std::vector<Json::Node>& base_requests);
    void BuildRequestRouter(const std::vector<Json::Node>& stat_requests);
};
