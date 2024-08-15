#pragma once
#include "city_router.h"
#include "router.h"

class Builder {
    RoutingSetting route_setting {};
    std::unique_ptr<CityRouter> city_router = std::make_unique<CityRouter>();
    std::shared_ptr<Graph::DirectedWeightedGraph<double>> dwg = nullptr;
    std::unique_ptr<Graph::Router<double>> router=nullptr;
    std::vector<Json::Node> requests;

public:
    Builder() = default;
    void ParseData(std::istream& input = std::cin);
    void PrintRequest(std::ostream& output = std::cout);

private:
    void BuildCityRouter(std::vector<Json::Node> const& base_requests);
    void BuildGraph();

    void BuildRequestRouter(std::vector<Json::Node> const& stat_requests);
    void MakeRequestRouteJson(const Json::Dict& dict);
};
