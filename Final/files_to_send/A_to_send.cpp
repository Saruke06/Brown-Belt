#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <cmath>
#include <stdexcept>
#include <algorithm>


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

    static Stop ParseFrom(std::string_view input) {
        std::string name = std::string(ReadToken(input, ": "));
        const double latitude = ConvertToDouble(ReadToken(input, ", "));
        const double longitude = ConvertToDouble(input);
        return {name, latitude, longitude};
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

private:
    std::string stop_name;
    std::optional<double> latitude;
    std::optional<double> longitude;

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

    void AddStop(const Stop& stop) {
        auto stop_name = stop.GetName();
        auto it_stop = map_of_stops.find(stop_name);
        if (it_stop == map_of_stops.end()) {
            map_of_stops[stop_name] = std::move(stop);
        } else {
            bool has_coordinates = it_stop->second.GetLatitude().has_value() && it_stop->second.GetLongitude().has_value();
            if (!has_coordinates) {
                if (stop.GetLatitude().has_value() && stop.GetLongitude().has_value()) {
                    it_stop->second.SetLatitude(stop.GetLatitude().value());
                    it_stop->second.SetLongitude(stop.GetLongitude().value());
                }
            }
        }
    }

    void AddBus(const std::string& bus_number, const std::vector<std::string>& stops_on_route) {
        for (const auto& stop : stops_on_route) {
            AddStop(stop);
        }
        buses[bus_number] = Bus(move(bus_number), move(stops_on_route));
    }

    std::string GetStopInfo(const std::string& stop_name) const {
        std::stringstream res;
        res << std::fixed << std::setprecision(6);
        auto it = map_of_stops.find(stop_name);
        if (it != map_of_stops.end()) {
            res << "Stop " << stop_name << " exists with latitude ";
            if (it->second.GetLatitude().has_value()) {
                res << *(it->second.GetLatitude());
            } else {
                res << "unknown";
            }
            res << " and longitude ";
            if (it->second.GetLongitude().has_value()) {
                res << *(it->second.GetLongitude());
            } else {
                res << "unknown";
            }
        } else {
            res << "NO stop " << stop_name << " in the DB";
        }
        return res.str();
    }

    std::string GetBusInfo(const std::string& bus_number) const {
        // output = "Bus X: R stops on route, U unique stops, L route length"
        std::stringstream ss;
        ss << "Bus " << bus_number << ": ";

        if (buses.count(bus_number) == 0) {
            ss << "not found";
        } else {
            auto route = buses.at(bus_number).GetRoute();
            ss << route.size() << " stops on route, ";
            std::unordered_set<std::string> unique_stops(route.begin(), route.end());
            // for (const auto& stop : route) {
            //     unique_stops.insert(stop);
            // }
            ss << unique_stops.size() << " unique stops, ";
            double route_length = 0.;
            for (size_t i = 1; i < route.size(); ++i) {
                double distance = ComputeDistance(map_of_stops.at(route[i - 1]), map_of_stops.at(route[i]));
                route_length += distance;
            }
            ss << route_length << " route length";
        }
        return ss.str();
    }

    Bus GetBus(const std::string& bus_number) const {
        return buses.at(bus_number);
    }

    static double ComputeDistance(const Stop& lhs, const Stop& rhs) {
        if (!lhs.GetLatitude() || !lhs.GetLongitude() || !rhs.GetLatitude() || !rhs.GetLongitude()) {
            std::cerr << "NO coordinates" << std::endl;
        }

        double lhs_lat = toRadians(*lhs.GetLatitude());
        double rhs_lat = toRadians(*rhs.GetLatitude());
        double lhs_lon = toRadians(*lhs.GetLongitude());
        double rhs_lon = toRadians(*rhs.GetLongitude());

        // prent(*lhs.GetLatitude());
        // prent(lhs_lat);
        // prent(sin(lhs_lat));
        // prent(cos(lhs_lat));
        // prent(*rhs.GetLatitude());
        // prent(rhs_lat);
        // prent(sin(rhs_lat));
        // prent(cos(rhs_lat));
        // prent(*lhs.GetLongitude());
        // prent(lhs_lon);
        // prent(sin(lhs_lon));
        // prent(cos(lhs_lon));
        // prent(*rhs.GetLongitude());
        // prent(rhs_lon);
        // prent(sin(rhs_lon));
        // prent(cos(rhs_lon));

        // double SinSin = sin(lhs_lat) * sin(rhs_lat);
        // double CosCos = cos(lhs_lat) * cos(rhs_lat);
        // double Cos = cos(abs(lhs_lon - rhs_lon));
        // double CosCosCos = CosCos * Cos;
        // double Acos = acos(SinSin + CosCosCos);

        // prent(SinSin);
        // prent(CosCos);
        // prent(Cos);
        // prent(CosCosCos);
        // prent(Acos);

        // double should = Acos * EARTH_RADIUS;

        // double ret = acos(sin(lhs_lat) * sin(rhs_lat) + 
        //             cos(lhs_lat) * cos(rhs_lat) *
        //             cos(abs(lhs_lon - rhs_lon))) * EARTH_RADIUS;

        // prent(should);
        // prent(ret);

        return acos(sin(lhs_lat) * sin(rhs_lat) + 
                    cos(lhs_lat) * cos(rhs_lat) *
                    cos(fabs(lhs_lon - rhs_lon))) * EARTH_RADIUS;
    }

private:
    std::unordered_map<std::string, Bus> buses;
    std::unordered_map<std::string, Stop> map_of_stops;

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
        OUT_BUS
    };

    Request(Type type) : type(type) {}
    static RequestHolder Create(Request::Type type);
    virtual void ParseFrom(std::string_view input) = 0;
    virtual ~Request() = default;

    const Type type;
};

struct ModifyRequest : Request {
    using Request::Request;
    virtual void Process(TransportDatabase& db) const = 0;
};

struct AddBusRequest : ModifyRequest {
    AddBusRequest() : ModifyRequest(Type::ADD_BUS) {}

    void ParseFrom(std::string_view input) override {
        bus_number = std::string(ReadToken(input, ": "));
        stops = ParseStops(input);
    }

    void Process(TransportDatabase& db) const override {
        db.AddBus(std::move(bus_number), std::move(stops));
    }

    std::string bus_number;
    std::vector<std::string> stops;
};

struct AddStopRequest : ModifyRequest {
    AddStopRequest() : ModifyRequest(Type::ADD_STOP) {}

    void ParseFrom(std::string_view input) override {
        // input = "X: latitude, longitude"
        stop = Stop::ParseFrom(input);
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
};

struct BusRequest : ReadRequest<std::string> {
    BusRequest() : ReadRequest(Type::OUT_BUS) {}

    void ParseFrom(std::string_view input) override {
        // input = "X"
        bus_number = std::string(input);
    }

    std::string Process(const TransportDatabase& db) const override {
        // return db.GetBus();
        return db.GetBusInfo(bus_number);
    }

    std::string bus_number;
};

RequestHolder Request::Create(Request::Type type) {
    switch (type) {
        case Type::ADD_STOP:
            return std::make_unique<AddStopRequest>();
        case Type::ADD_BUS:
            return std::make_unique<AddBusRequest>();
        case Type::OUT_BUS:
            return std::make_unique<BusRequest>();
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
        }
    }
    return std::nullopt;
}

RequestHolder ParseRequest(std::string_view request_str) {
    bool is_add_request = request_str.find(":") != std::string::npos;
    const auto request_type = ConvertRequestTypeFromString(ReadToken(request_str), is_add_request);
    if (!request_type) {
        return nullptr;
    }

    RequestHolder request = Request::Create(*request_type);
    if (request) {
        request->ParseFrom(request_str);
    }
    return request;
}

std::vector<RequestHolder> ReadRequests(std::istream& input = std::cin) {
    const size_t request_count = ReadNumberOnLine<size_t>(input);
    std::vector<RequestHolder> updates;
    updates.reserve(request_count);

    for (size_t i = 0; i < request_count; ++i) {
        std::string request_str;
        getline(input, request_str);
        if (auto request = ParseRequest(request_str)) {
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

std::vector<std::string> ProcessRequests(const TransportDatabase& db, const std::vector<RequestHolder>& requests) {
    std::vector<std::string> responses;

    for (const auto& request_holder : requests) {
        if (request_holder->type == Request::Type::OUT_BUS) {
            const auto& request = static_cast<const BusRequest&>(*request_holder);
            responses.push_back(request.Process(db));
        }
        // } else {
        //     const auto& request = static_cast<const ModifyRequest&>(*request_holder);
        //     request.Process(db);
        // }
    }

    return responses;
}

void PrintResponses(const std::vector<std::string>& responses, std::ostream& output = std::cout) {
    for (const std::string& response : responses) {
        output << response << '\n';
    }
}

// #include "test_string_parses.h"
// #include "test_requests.h"
// #include "test_transport_db.h"

using namespace std;


int main() {
    std::cerr << std::fixed << std::setprecision(6);
    std::cout << std::fixed << std::setprecision(6);
    //TestAllStringParses();
    //TestAllRequests();
    //TestAllTransportDB();

    TransportDatabase db;
    const auto modify_requests = ReadRequests();
    ProcessModifyRequests(&db, modify_requests);
    const auto read_requests = ReadRequests();
    const auto responses = ProcessRequests(db, read_requests);
    PrintResponses(responses);

    return 0;
}