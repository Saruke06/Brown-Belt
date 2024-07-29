#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <set>
#include <map>
#include <cmath>
#include <unordered_set>
#include <variant>

using namespace std;

//#include "json.h"
namespace Json {

  class Node;
  using Dict = std::map<std::string, Node>;

  class Node : std::variant<std::vector<Node>, Dict, bool, int, double, std::string> {
  public:
    using variant::variant;
    const variant& GetBase() const { return *this; }

    const auto& AsArray() const { return std::get<std::vector<Node>>(*this); }
    const auto& AsMap() const { return std::get<Dict>(*this); }
    bool AsBool() const { return std::get<bool>(*this); }
    int AsInt() const { return std::get<int>(*this); }
    double AsDouble() const { 
        return std::holds_alternative<double>(*this) ? std::get<double>(*this) : std::get<int>(*this);
    }
    const auto& AsString() const { return std::get<std::string>(*this); }
  };

  class Document {
  public:
    explicit Document(Node root) : root(std::move(root)) {}

    const Node& GetRoot() const {
      return root;
    }

  private:
    Node root;
  };


  Node LoadNode(istream& input);

  Node LoadArray(istream& input) {
    vector<Node> result;

    for (char c; input >> c && c != ']'; ) {
      if (c != ',') {
        input.putback(c);
      }
      result.push_back(LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadBool(istream& input) {
    string s;
    while (isalpha(input.peek())) {
      s.push_back(input.get());
    }
    return Node(s == "true");
  }

  Node LoadNumber(istream& input) {
    bool is_negative = false;
    if (input.peek() == '-') {
      is_negative = true;
      input.get();
    }
    int int_part = 0;
    while (isdigit(input.peek())) {
      int_part *= 10;
      int_part += input.get() - '0';
    }
    if (input.peek() != '.') {
      return Node(int_part * (is_negative ? -1 : 1));
    }
    input.get();  // '.'
    double result = int_part;
    double frac_mult = 0.1;
    while (isdigit(input.peek())) {
      result += frac_mult * (input.get() - '0');
      frac_mult /= 10;
    }
    return Node(result * (is_negative ? -1 : 1));
  }

  Node LoadString(istream& input) {
    string line;
    getline(input, line, '"');
    return Node(move(line));
  }

  Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}'; ) {
      if (c == ',') {
        input >> c;
      }

      string key = LoadString(input).AsString();
      input >> c;
      result.emplace(move(key), LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
      return LoadArray(input);
    } else if (c == '{') {
      return LoadDict(input);
    } else if (c == '"') {
      return LoadString(input);
    } else if (c == 't' || c == 'f') {
      input.putback(c);
      return LoadBool(input);
    } else {
      input.putback(c);
      return LoadNumber(input);
    }
  }

  Document Load(istream& input = cin) {
    return Document{LoadNode(input)};
  }
}

//#include "string_parses.h"
template <typename Number>
Number ReadNumberOnLine(std::istream& is) {
    Number result;
    is >> result;
    std::string dummy;
    getline(is, dummy);
    return result;
}

std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(std::string_view s, std::string_view delimeter = " ") {
    const size_t pos = s.find(delimeter);
    if (pos == s.npos) {
        return {s, std::nullopt};
    } else {
        return {s.substr(0, pos), s.substr(pos + delimeter.length())};
    }
}

std::pair<std::string_view, std::string_view> SplitTwo(std::string_view s, std::string_view delimeter = " ") {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimeter);
    return {lhs, rhs_opt.value_or("")};
}

std::string_view ReadToken(std::string_view& s, std::string_view delimeter = " ") {
    const auto [lhs, rhs] = SplitTwo(s, delimeter);
    s = rhs;
    return lhs;
}

double ConvertToDouble(std::string_view str) {
    size_t pos;
    const double result = std::stod(std::string(str), &pos);
    if (pos != str.length()) {
        std::stringstream error;
        error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
        throw std::invalid_argument(error.str());
    }
    return result;
}

template <typename Number>
Number ConvertToInt(std::string_view str) {
    size_t pos;
    const int result = std::stoi(std::string(str), &pos);
    if (pos != str.length()) {
        std::stringstream error;
        error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
        throw std::invalid_argument(error.str());
    }
    return result;
}

template <typename Number>
void ValidateBounds(Number value_to_check, Number min_value, Number max_value) {
    if (value_to_check <= min_value || value_to_check >= max_value) {
        std::stringstream error;
        error << value_to_check << " is out of bounds [" << min_value << ", " << max_value << "]";
        throw std::out_of_range(error.str());
    }
}

//#include "transport_db.h"
const double PI = 3.1415926535;
const double EARTH_RADIUS = 6371000;

class Stop {
public:

    Stop() = default;
    Stop(std::string name) {
        stop_name = name;
    }

    static Stop ParseFrom(const Json::Node& request_node) {
        // input = "X: latitude, longitude, D1m to stop1, D2m to stop2, ..."
        std::string name = request_node.AsMap().at("name").AsString();
        const double latitude = request_node.AsMap().at("latitude").AsDouble();
        const double longitude = request_node.AsMap().at("longitude").AsDouble();
        if(request_node.AsMap().count("road_distances") == 0) {
            return {name, latitude, longitude};
        } else {
            Stop stop(name, latitude, longitude);
            for (const auto& [stop_name, distance] : request_node.AsMap().at("road_distances").AsMap()) {
                stop.AddDistance(stop_name, distance.AsInt());
            }
            return stop;
        }
    }

    void SetLatitude(double lat) {
        latitude = lat;
    }

    void SetLongitude(double lon) {
        longitude = lon;
    }

    std::string GetName() const {
        return stop_name;
    }

    std::optional<double> GetLatitude() const {
        return latitude;
    }

    std::optional<double> GetLongitude() const {
        return longitude;
    }

    void AddBusToStop(const std::string& bus_number) {
        buses_for_stop.insert(bus_number);
    }

    const std::set<std::string>& GetBuses() const {
        return buses_for_stop;
    }

    void AddDistance(const std::string& stop_name, int distance) {
        distances[stop_name] = distance;
    }

    void AddDistances(const std::unordered_map<std::string, int>& dist) {
        distances = std::move(dist);
    }

    std::optional<int> GetDistance(const std::string& stop_name) const {
        if (distances.count(stop_name) == 0) {
            return std::nullopt;
        }
        return distances.at(stop_name);
    }

    const auto& GetDistances() const {
        return distances;
    }

private:
    std::string stop_name;
    std::optional<double> latitude;
    std::optional<double> longitude;
    std::set<std::string> buses_for_stop;
    std::unordered_map<std::string, int> distances;

    Stop(std::string name, double latitude, double longitude) 
        : stop_name(name), latitude(latitude), longitude(longitude) {}
};

class Bus {
public:
    Bus() = default;
    Bus(std::string number, std::vector<std::string> stops) 
        : bus_number(number) {
            for (auto& stop : stops)
                route.push_back(std::move(stop));
        }

    std::vector<std::string> GetRoute() const {
        return route;
    }

private:
    std::string bus_number;
    std::vector<std::string> route;
};

class TransportDatabase {
public:
    TransportDatabase() = default;

    void CheckAndSetStopCoordinates(const Stop& stop) {
        auto it_stop = stops.find(stop.GetName());
        bool has_coordinates = it_stop->second->GetLatitude().has_value() && it_stop->second->GetLongitude().has_value();
        if (!has_coordinates) {
            if (stop.GetLatitude().has_value() && stop.GetLongitude().has_value()) {
                it_stop->second->SetLatitude(stop.GetLatitude().value());
                it_stop->second->SetLongitude(stop.GetLongitude().value());
            }
        }
    }

    void AddReverseDistances(const Stop& stop) {
        for (const auto& [stop_name, distance] : stop.GetDistances()) {
            auto it = stops.find(stop_name);
            if (it != stops.end()) {
                it->second->AddDistance(stop.GetName(), distance);
            } else {
                stops[stop_name] = std::shared_ptr<Stop>(new Stop(stop_name));
                stops[stop_name]->AddDistance(stop.GetName(), distance);
            }
        }
    }

    void AddStop(const Stop& stop) {
        auto current_stop_name = stop.GetName();
        auto it_stop = stops.find(current_stop_name);
        if (it_stop == stops.end()) {
            stops[current_stop_name] = std::shared_ptr<Stop>(new Stop(stop));
            //AddReverseDistances(stop);

        } else {
            CheckAndSetStopCoordinates(stop);

            //AddReverseDistances(stop);

            // Merge stop.GetDistances() to stops[current_stop_name] without overwriting
            for (const auto& [stop_name, distance] : stop.GetDistances()) {
                if (it_stop->second->GetDistances().count(stop_name) == 0) {
                    it_stop->second->AddDistance(stop_name, distance);
                }
            }
        }
    }

    void AddBus(const std::string& bus_number, const std::vector<std::string>& stops_on_route) {
        for (const auto& stop : stops_on_route) {
            AddStop(stop);
            stops[stop]->AddBusToStop(bus_number);
        }
        buses[bus_number] = std::shared_ptr<Bus>(new Bus(move(bus_number), move(stops_on_route)));
    }

    Json::Node GetStopInfoJson(const std::string& stop_name, int id) const {
        // Example: 
        /*      
            "buses": [
              "256",
              "828"
            ],
            "request_id": 1042838872
        */
        std::vector<Json::Node> buses;
        auto it = stops.find(stop_name);
        if (it != stops.end()) {
            for (const auto& bus : it->second->GetBuses()) {
                buses.push_back(Json::Node{std::string(bus)});
            }
            return Json::Node(std::map<std::string, Json::Node>{
                {std::string("buses"), buses}, 
                {std::string("request_id"), id}
            });
        } else {
            return Json::Node(std::map<std::string, Json::Node>{
                {std::string("error_message"), std::string("not found")},
                {std::string("request_id"), id}
            });
        }
    }

    Json::Node GetBusInfoJson(const std::string& bus_number, int id) const {
        // Rebuild as JSON
        // Example:
        /* 
            "route_length": 27600,
            "request_id": 519139350,
            "curvature": 1.31808,
            "stop_count": 5,
            "unique_stop_count": 3
        */
        
        if (buses.count(bus_number) == 0) {
            return Json::Node(std::map<std::string, Json::Node>{
                {"error_message", std::string("not found")},
                {"request_id", id}
            });
        } else {
            auto route = buses.at(bus_number)->GetRoute();
            std::unordered_set<std::string> unique_stops(route.begin(), route.end());
            auto [route_length, fact_route_length] = ComputeRouteAndFactRouteLength(bus_number);
            double c = fact_route_length / route_length;
            return Json::Node(std::map<std::string, Json::Node>{
                {"route_length", fact_route_length},
                {"request_id", id},
                {"curvature", c},
                {"stop_count", int(route.size())},
                {"unique_stop_count", int(unique_stops.size())}
            });
        }
    }

    std::string GetStopInfo(const std::string& stop_name, int id) const {
        // Example: 
        /*      
            "buses": [
              "256",
              "828"
            ],
            "request_id": 1042838872
        */
        std::vector<std::string> buses;
        std::ostringstream os;
        auto it = stops.find(stop_name);
        if (it != stops.end()) {
            for (const auto& bus : it->second->GetBuses()) {
                buses.push_back(std::string(bus));
            }
            if (!buses.empty()) {
                os << "{\n";
                os << "  \"buses\": [\n";
                for (size_t i = 0; i < buses.size(); ++i) {
                    os << "    \"" << buses[i] << "\"";
                    if (i != buses.size() - 1) {
                        os << ",\n";
                    } else {
                        os << "\n";
                    }
                }
                os << "  ],\n";
                os << "  \"request_id\": " << id << "\n";
                os << "}";
            } else {
                os << "{\n";
                os << "  \"buses\": [],\n";
                os << "  \"request_id\": " << id << "\n";
                os << "}";
            }
        } else {
            os << "{\n";
            os << "  \"error_message\": \"not found\",\n";
            os << "  \"request_id\": " << id << "\n";
            os << "}";
        }
        return os.str();
    }

    std::string GetBusInfo(const std::string& bus_number, int id) const {
        // Rebuild as JSON
        // Example:
        /* 
            "route_length": 27600,
            "request_id": 519139350,
            "curvature": 1.31808,
            "stop_count": 5,
            "unique_stop_count": 3
        */
        
        std::ostringstream os;
        if (buses.count(bus_number) == 0) {
            os << "{\n";
            os << "  \"error_message\": \"not found\",\n";
            os << "  \"request_id\": " << id << "\n";
            os << "}";
            return os.str();
        } else {
            auto route = buses.at(bus_number)->GetRoute();
            std::unordered_set<std::string> unique_stops(route.begin(), route.end());
            auto [route_length, fact_route_length] = ComputeRouteAndFactRouteLength(bus_number);
            double c = fact_route_length / route_length;
            os << "{\n";
            os << "  \"route_length\": " << fact_route_length << ",\n";
            os << "  \"request_id\": " << id << ",\n";
            os << "  \"curvature\": " << std::fixed << std::setprecision(6) << c << ",\n";
            os << "  \"stop_count\": " << route.size() << ",\n";
            os << "  \"unique_stop_count\": " << unique_stops.size() << "\n";
            os << "}";
            return os.str();
        }
    }


    const auto& GetBus(const std::string& bus_number) const {
        return buses.at(bus_number);
    }

    const auto& GetStop(const std::string& stop_name) const {
        return stops.at(stop_name);
    }

    static double ComputeDistance(const Stop& lhs, const Stop& rhs) {
        if (!lhs.GetLatitude() || !lhs.GetLongitude() || !rhs.GetLatitude() || !rhs.GetLongitude()) {
            std::cerr << "NO coordinates" << std::endl;
        }

        double lhs_lat = toRadians(*lhs.GetLatitude());
        double rhs_lat = toRadians(*rhs.GetLatitude());
        double lhs_lon = toRadians(*lhs.GetLongitude());
        double rhs_lon = toRadians(*rhs.GetLongitude());

        return acos(sin(lhs_lat) * sin(rhs_lat) + 
                    cos(lhs_lat) * cos(rhs_lat) *
                    cos(fabs(lhs_lon - rhs_lon))) * EARTH_RADIUS;
    }

    std::pair<double, int> ComputeRouteAndFactRouteLength(const std::string& bus_number) const {
        double route_length = 0.;
        int fact_route_length = 0.;
        const auto& route = buses.at(bus_number)->GetRoute();
        for (size_t i = 1; i < route.size(); ++i) {
            route_length += ComputeDistance(*stops.at(route[i - 1]), *stops.at(route[i]));
            fact_route_length += FindDistance(route[i - 1], route[i]);
        }
        return {route_length, fact_route_length};
    }

    int FindDistance(const std::string& stop1, const std::string& stop2) const {
        // если для stop1 нет расстояния до stop2, то вернуть расстояние от stop2 до stop1
        auto it = stops.find(stop1);
        if (it != stops.end()) {
            auto distance = it->second->GetDistance(stop2);
            if (distance.has_value()) {
                return distance.value();
            }
        }
        return stops.at(stop2)->GetDistance(stop1).value();
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Bus>> buses;
    std::unordered_map<std::string, std::shared_ptr<Stop>> stops;

    static double toRadians(double degree) {
        return degree * PI / 180.0;
    }
};

std::vector<std::string> ParseStops(std::string_view input) {
    // 1. stop1 - stop2 - ... - stopN: автобус следует от stop1 до stopN и обратно с указанными промежуточными остановками.
    // 2. stop1 > stop2 > ... > stopN > stop1: кольцевой маршрут с конечной stop1.
    // input = "256: stop1 - stop2 - ... - stopN"

    bool is_ring = input.find(" > ") != std::string::npos;
    std::vector<std::string> stops;
    
    std::string_view delimiter = is_ring ? " > " : " - ";

    while (!input.empty()) {
        stops.push_back(std::string(ReadToken(input, delimiter)));
    }

    // if !is_ring then add stops in reverse order
    if (!is_ring) {
        for (size_t i = stops.size() - 1; i > 0; --i) {
            stops.push_back(stops[i - 1]);
        }
    }

    return stops;
}

//#include "requests.h"
struct Request;
using RequestHolder = std::unique_ptr<Request>;

struct Request {
    enum class Type {
        ADD_STOP,
        ADD_BUS,
        OUT_BUS,
        OUT_STOP
    };

    Request(Type type) : type(type) {}
    static RequestHolder Create(Request::Type type);
    virtual void ParseFrom(const Json::Node& request_node) = 0;
    virtual ~Request() = default;

    const Type type;
};

struct ModifyRequest : Request {
    using Request::Request;
    virtual void Process(TransportDatabase& db) const = 0;
    virtual ~ModifyRequest() = default;
};

struct AddBusRequest : ModifyRequest {
    AddBusRequest() : ModifyRequest(Type::ADD_BUS) {}
    virtual ~AddBusRequest() = default;

    std::vector<std::string> NodesToStrings(const std::vector<Json::Node>& nodes, bool is_roundtrip) const {
        std::vector<std::string> result;
        for (const Json::Node& node : nodes) {
            result.push_back(node.AsString());
        }

        if (!is_roundtrip) {
            for (size_t i = result.size() - 1; i > 0; --i) {
                result.push_back(result[i - 1]);
            }
        }
        return result;
    }

    void ParseFrom(const Json::Node& request_node) override {
        bus_number = request_node.AsMap().at("name").AsString();
        bool is_roundtrip = request_node.AsMap().at("is_roundtrip").AsBool();
        
        stops = NodesToStrings(request_node.AsMap().at("stops").AsArray(), is_roundtrip);
    }

    void Process(TransportDatabase& db) const override {
        db.AddBus(std::move(bus_number), std::move(stops));
    }

    std::string bus_number;
    std::vector<std::string> stops;
};

struct AddStopRequest : ModifyRequest {
    AddStopRequest() : ModifyRequest(Type::ADD_STOP) {}
    virtual ~AddStopRequest() = default;

    void ParseFrom(const Json::Node& request_node) override {
        stop = Stop::ParseFrom(request_node);
    }

    void Process(TransportDatabase& db) const override {
        // Add stop to db
        db.AddStop(std::move(stop));
    }

    Stop stop;
};

template <typename ResultType>
struct ReadRequest : Request {
    using Request::Request;
    virtual ResultType Process(const TransportDatabase& db) const = 0;
    virtual ~ReadRequest() = default;
};

struct BusRequest : ReadRequest<std::string> {
    BusRequest() : ReadRequest(Type::OUT_BUS) {}
    virtual ~BusRequest() = default;

    void ParseFrom(const Json::Node& request_node) override {
        // input = "X"
        id = request_node.AsMap().at("id").AsInt();
        bus_number = request_node.AsMap().at("name").AsString();
    }

    std::string Process(const TransportDatabase& db) const override {
        return db.GetBusInfo(bus_number, id);
    }

    std::string bus_number;
    int id = 0;
};

struct StopRequest : ReadRequest<std::string> {
    StopRequest() : ReadRequest(Type::OUT_STOP) {}
    virtual ~StopRequest() = default;

    void ParseFrom(const Json::Node& request_node) override {
        // input = "X"
        id = request_node.AsMap().at("id").AsInt();
        stop_name = request_node.AsMap().at("name").AsString();
    }

    std::string Process(const TransportDatabase& db) const override {
        return db.GetStopInfo(stop_name, id);
    }

    std::string stop_name;
    int id = 0;
};

RequestHolder Request::Create(Request::Type type) {
    switch (type) {
        case Type::ADD_STOP:
            return std::make_unique<AddStopRequest>();
        case Type::ADD_BUS:
            return std::make_unique<AddBusRequest>();
        case Type::OUT_BUS:
            return std::make_unique<BusRequest>();
        case Type::OUT_STOP:
            return std::make_unique<StopRequest>();
        default:
            return nullptr;
    }
}

std::optional<Request::Type> ConvertRequestTypeFromString(std::string_view type_str, bool is_add_request) {
    if (is_add_request) {
        if (type_str == "Bus") {
            return Request::Type::ADD_BUS;
        } else if (type_str == "Stop") {
            return Request::Type::ADD_STOP;
        }
    } else {
        if (type_str == "Bus") {
            return Request::Type::OUT_BUS;
        } else if (type_str == "Stop") {
            return Request::Type::OUT_STOP;
        }
    }
    return std::nullopt;
}

RequestHolder ParseRequest(const Json::Node& request_node, bool is_add_request) {
    const auto request_type = ConvertRequestTypeFromString(request_node.AsMap().at("type").AsString(), is_add_request);
    if (!request_type) {
        return nullptr;
    }

    RequestHolder request = Request::Create(*request_type);
    if (request) {
        request->ParseFrom(request_node);
    }
    return request;
}

std::vector<RequestHolder> ReadRequests(const std::vector<Json::Node>& requests, bool is_add_request) {
    const size_t request_count = requests.size();
    std::vector<RequestHolder> updates;
    updates.reserve(request_count);

    for (size_t i = 0; i < request_count; ++i) {
        if (auto request = ParseRequest(requests[i], is_add_request)) {
            updates.push_back(std::move(request));
        }
    }
    return updates;
}

void ProcessModifyRequests(TransportDatabase* db = nullptr, const std::vector<RequestHolder>& requests = {}) {
    if (!db) {
        throw std::invalid_argument("db is nullptr");
    }
    for (const auto& request_holder : requests) {
        const auto& request = static_cast<const ModifyRequest&>(*request_holder);
        request.Process(*db);
    }
}

std::vector<Json::Node> ProcessRequestsJson(const TransportDatabase& db, const std::vector<RequestHolder>& requests) {
    std::vector<Json::Node> responses;

    for (const auto& request_holder : requests) {
        switch(request_holder->type) {
            case Request::Type::OUT_BUS:
            case Request::Type::OUT_STOP: {
                const auto& request = static_cast<const ReadRequest<Json::Node>&>(*request_holder);
                responses.push_back(request.Process(db));
                break;
            }
            default:
                throw std::invalid_argument("Unknown request type");
        }
    }

    return responses;
}

std::vector<std::string> ProcessRequests(const TransportDatabase& db, const std::vector<RequestHolder>& requests) {
    std::vector<std::string> responses;

    for (const auto& request_holder : requests) {
        switch(request_holder->type) {
            case Request::Type::OUT_BUS:
            case Request::Type::OUT_STOP: {
                const auto& request = static_cast<const ReadRequest<std::string>&>(*request_holder);
                responses.push_back(request.Process(db));
                break;
            }
            default:
                throw std::invalid_argument("Unknown request type");
        }
    }

    return responses;
}

// void PrintResponsesJson(const std::vector<Json::Node>& responses, std::ostream& output = std::cout) {
//     output << responses;
//     // for (const auto& response : responses) {
//     //     output << response << std::endl;
//     // }
// }

void PrintResponses(const std::vector<std::string>& responses, std::ostream& output = std::cout) {
    // вывести все элементы векторатора respo вмежду э, каждый элемент выводить на новой строке
    // между элементами вставить запятую и пробел
    bool first = true;
    output << "[\n";
    for (const auto& response : responses) {
        if (first) {
            output << response;
            first = false;
        } else {
            output << ", \n" << response;
        }
    }
    output << "\n]";
}

using namespace Json;

// #include "test_string_parses.h"
// #include "test_requests.h"
// #include "test_transport_db.h"

int main() {
    // TestAllStringParses();
    // TestAllRequests();
    // TestAllTransportDB();
    // TestAllJson();

    TransportDatabase db;
    Document doc = Load();
    const auto modify_requests = ReadRequests(doc.GetRoot().AsMap().at("base_requests").AsArray(), true);
    ProcessModifyRequests(&db, modify_requests);
    const auto read_requests = ReadRequests(doc.GetRoot().AsMap().at("stat_requests").AsArray(), false);

    const auto responses = ProcessRequests(db, read_requests);
    PrintResponses(responses);

    return 0;
}