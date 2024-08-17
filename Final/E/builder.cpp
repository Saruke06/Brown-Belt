#include "builder.h"

using namespace std::string_literals;

void Builder::ParseData(std::istream& input) {
    auto rootNodeMap = Json::Load(input).GetRoot().AsMap();
    // auto route_set = rootNodeMap.at("routing_settings").AsMap();
    // route_setting = RoutingSetting(route_set.at("bus_wait_time").AsDouble(),
    //     route_set.at("bus_velocity").AsDouble());
    auto base_requests = rootNodeMap.at("base_requests").AsArray();
    auto stat_requests = rootNodeMap.at("stat_requests").AsArray();
    BuildCityRouter(base_requests);
    BuildRequestRouter(stat_requests);
}

void Builder::BuildCityRouter(const std::vector<Json::Node>& base_requests) {
    for (const auto& current_base : base_requests) {
        auto& currnod = current_base.AsMap();
        if (currnod.at("type").AsString() == "Stop"s) {
            city_router->AddStop(currnod);
        } else if (currnod.at("type").AsString() == "Bus"s) {
            city_router->AddBus(currnod);
        }
    }
}

void Builder::BuildRequestRouter(const std::vector<Json::Node>& stat_requests) {
    for (const auto& current_request : stat_requests) {
        auto& request_body = current_request.AsMap();
        if (request_body.at("type").AsString() == "Stop"s) {
            requests.emplace_back(city_router->GetInfoForStop(request_body));
        } else if (request_body.at("type").AsString() == "Bus"s) {
            requests.emplace_back(city_router->GetInfoForBus(request_body));
        } else if (request_body.at("type").AsString() == "Route"s) {
            // 
        }    
    }
}

void Builder::PrintRequest(std::ostream& output) {
    Json::Print(Json::Document(requests), output);
}
