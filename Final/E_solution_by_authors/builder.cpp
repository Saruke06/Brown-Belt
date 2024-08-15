#include "builder.h"

using namespace std::string_literals;
void Builder::ParseData(std::istream& input)
{
    auto rootNodeMap = Json::Load(input).GetRoot().AsMap();
    auto rout_set = rootNodeMap.at("routing_settings").AsMap();
    route_setting = RoutingSetting(rout_set.at("bus_wait_time").AsDouble(),
        rout_set.at("bus_velocity").AsDouble());
    auto base_requests = rootNodeMap.at("base_requests").AsArray();
    auto stat_requests = rootNodeMap.at("stat_requests").AsArray();
	BuildCityRouter(base_requests);
	BuildGraph();
	BuildRequestRouter(stat_requests);
}

void Builder::BuildCityRouter(std::vector<Json::Node> const& base_requests)
{
    for (auto const& current_base : base_requests) {
        auto& currnod = current_base.AsMap();
        if (currnod.at("type").AsString() == "Stop"s) {
            city_router->AddStop(currnod);
        } else if (currnod.at("type").AsString() == "Bus"s) {
            city_router->AddBus(currnod);
        }
    }
}

void Builder::BuildGraph()
{
    auto const& ALLSTOPS = city_router->all_stops;
    dwg = std::make_shared<Graph::DirectedWeightedGraph<double>>(ALLSTOPS.size() * 2);
    for (const auto& [name, stop] : ALLSTOPS) {
        dwg->AddEdge({ stop->index,
            stop->index + ALLSTOPS.size(),
            route_setting.bus_wait_time,
            Graph::EDGE_TYPE::WAIT,
            stop->st_name });
    }
    auto const& ALLBUSES = city_router->all_buses;
    for (const auto& [name, bus] : ALLBUSES) {
        auto bus_stops = bus->stops;
        auto const END = bus_stops.end();
        for (auto stop_it = std::begin(bus_stops); next(stop_it) != END; ++stop_it) {
            size_t span = 0;
            int dist = 0;
            for (auto next_stop_it = next(stop_it); next_stop_it != END; ++next_stop_it) {
                dist += LinearDistanse(*prev(next_stop_it),
                    *next_stop_it);
				dwg->AddEdge({
					(*stop_it)->index + ALLSTOPS.size(),
					(*next_stop_it)->index,
					dist / route_setting.bus_velocity,
					Graph::EDGE_TYPE::BUS,
					bus->name_bs,
					++span
					});
            }
        }
    }
	router = std::make_unique<Graph::Router<double>>(std::move(*dwg));
}

void Builder::BuildRequestRouter(std::vector<Json::Node> const& stat_requests)
{
    for (const auto& current_request : stat_requests) {
        auto& request_body = current_request.AsMap();
        if (request_body.at("type").AsString() == "Stop"s) {
            requests.emplace_back(city_router->GetInfoForStop(request_body));
        } else if (request_body.at("type").AsString() == "Bus"s) {
            requests.emplace_back(city_router->GetInfoForBus(request_body));
        }
		else if (request_body.at("type").AsString() == "Route"s) {
			MakeRequestRouteJson(request_body);
		}
    }
}

void Builder::MakeRequestRouteJson(const Json::Dict & dict)
{
	Json::Dict request_dict;
	request_dict["request_id"] = dict.at("id").AsInt();
	auto fromstr = dict.at("from").AsString();
	auto tostr = dict.at("to").AsString();
	auto from = city_router->all_stops.at(fromstr)->index;
	auto to = city_router->all_stops.at(tostr)->index;
	auto route = router->BuildRoute(from, to);
	if (route.has_value()) {
		request_dict["total_time"] = Json::Node(route->weight);
		std::vector<Item> items;
		auto NUMs = route->edge_count;
		items.reserve(NUMs);
		for (size_t num = 0; num < NUMs; ++num) {
			auto edge_id = router->GetRouteEdge(route->id, num);
			auto edge = dwg->GetEdge(edge_id);
			items.emplace_back(
				edge.edge_type, edge.name, edge.weight, edge.span
			);
		}
		std::vector<Json::Node> vecitems;
		vecitems.reserve(items.size());
		for (auto& itemrequest : items) {
			Json::Dict reqdict;
			if (itemrequest.edge_type == Graph::EDGE_TYPE::BUS) {
				reqdict["type"] = Json::Node("Bus"s);
				reqdict["bus"] = Json::Node(itemrequest.name);
				reqdict["time"] = Json::Node(itemrequest.time);
				reqdict["span_count"] = Json::Node(static_cast<int>(itemrequest.span));
                                vecitems.emplace_back(move(reqdict));
                        }
			else {
				Json::Dict reqdict;
				reqdict["type"] = Json::Node("Wait"s);
				reqdict["stop_name"] = Json::Node(itemrequest.name);
				reqdict["time"] = Json::Node(itemrequest.time);
                                vecitems.emplace_back(move(reqdict));
                        }
		}
		request_dict["items"] = move(vecitems);
	}
	else {
		request_dict["error_message"] = Json::Node("not found"s);
	}
        requests.emplace_back(request_dict);
}

void Builder::PrintRequest(std::ostream& output)
{
	Json::Print(Json::Document(requests), output);
}
