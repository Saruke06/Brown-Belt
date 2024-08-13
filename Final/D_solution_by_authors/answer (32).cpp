#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <variant>
#include <vector>

using namespace std;

namespace Json {

  class Node;
  using Dict = map<string, Node>;

  class Node : variant<vector<Node>, Dict, bool, int, double, string> {
  public:
    using variant::variant;
    const variant& GetBase() const { return *this; }

    const auto& AsArray() const { return get<vector<Node>>(*this); }
    const auto& AsMap() const { return get<Dict>(*this); }
    bool AsBool() const { return get<bool>(*this); }
    int AsInt() const { return get<int>(*this); }
    double AsDouble() const { 
        return holds_alternative<double>(*this) ? get<double>(*this) : get<int>(*this);
    }
    const auto& AsString() const { return get<string>(*this); }
  };

  class Document {
  public:
    explicit Document(Node root) : root(move(root)) {}

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

  Document Load(istream& input) {
    return Document{LoadNode(input)};
  }

  void PrintNode(const Json::Node& node, ostream& output);

  template <typename Value>
  void PrintValue(const Value& value, ostream& output) {
    output << value;
  }

  template <>
  void PrintValue<string>(const string& value, ostream& output) {
    output << '"' << value << '"';
  }

  template <>
  void PrintValue<bool>(const bool& value, ostream& output) {
    output << boolalpha << value;
  }

  template <>
  void PrintValue<vector<Node>>(const vector<Node>& nodes, ostream& output) {
    output << '[';
    bool first = true;
    for (const Node& node : nodes) {
      if (!first) {
        output << ", ";
      }
      first = false;
      PrintNode(node, output);
    }
    output << ']';
  }

  template <>
  void PrintValue<Dict>(const Dict& dict, ostream& output) {
    output << '{';
    bool first = true;
    for (const auto& [key, node]: dict) {
      if (!first) {
        output << ", ";
      }
      first = false;
      PrintValue(key, output);
      output << ": ";
      PrintNode(node, output);
    }
    output << '}';
  }

  void PrintNode(const Json::Node& node, ostream& output) {
    visit([&output](const auto& value) { PrintValue(value, output); },
          node.GetBase());
  }

  void Print(const Document& document, ostream& output) {
    PrintNode(document.GetRoot(), output);
  }

}

template <typename It>
class Range {
public:
  using ValueType = typename std::iterator_traits<It>::value_type;

  Range(It begin, It end) : begin_(begin), end_(end) {}
  It begin() const { return begin_; }
  It end() const { return end_; }

private:
  It begin_;
  It end_;
};

template <typename C>
auto AsRange(const C& container) {
  return Range{begin(container), end(container)};
}

template <typename It>
size_t ComputeUniqueItemsCount(Range<It> range) {
  return unordered_set<typename Range<It>::ValueType>{
      range.begin(), range.end()
  }.size();
}

template <typename K, typename V>
const V* GetValuePointer(const unordered_map<K, V>& map, const K& key) {
  if (auto it = map.find(key); it != end(map)) {
    return &it->second;
  } else {
    return nullptr;
  }
}

string_view Strip(string_view line) {
  while (!line.empty() && isspace(line.front())) {
    line.remove_prefix(1);
  }
  while (!line.empty() && isspace(line.back())) {
    line.remove_suffix(1);
  }
  return line;
}

namespace Sphere {
  const double PI = 3.1415926535;

  double ConvertDegreesToRadians(double degrees) {
    return degrees * PI / 180.0;
  }

  const double EARTH_RADIUS = 6'371'000;

  struct Point {
    double latitude;
    double longitude;

    static Point FromDegrees(double latitude, double longitude) {
      return {
        ConvertDegreesToRadians(latitude),
        ConvertDegreesToRadians(longitude)
      };
    }
  };

  double Distance(Point lhs, Point rhs) {
    lhs = Point::FromDegrees(lhs.latitude, lhs.longitude);
    rhs = Point::FromDegrees(rhs.latitude, rhs.longitude);
    return acos(
      sin(lhs.latitude) * sin(rhs.latitude)
      + cos(lhs.latitude) * cos(rhs.latitude) * cos(abs(lhs.longitude - rhs.longitude))
    ) * EARTH_RADIUS;
  }
}

namespace Descriptions {
  struct Stop {
    string name;
    Sphere::Point position;
    unordered_map<string, double> distances;

    static Stop ParseFrom(const Json::Dict& attrs) {
      Stop stop = {
          .name = attrs.at("name").AsString(),
          .position = {
              .latitude = attrs.at("latitude").AsDouble(),
              .longitude = attrs.at("longitude").AsDouble(),
          }
      };
      if (attrs.count("road_distances") > 0) {
        for (const auto& [neighbour_stop, distance_node] : attrs.at("road_distances").AsMap()) {
          stop.distances[neighbour_stop] = distance_node.AsInt();
        }
      }
      return stop;
    }
  };

  vector<string> ParseStops(const vector<Json::Node>& stop_nodes, bool is_roundtrip) {
    vector<string> stops;
    stops.reserve(stop_nodes.size());
    for (const Json::Node& stop_node : stop_nodes) {
      stops.push_back(stop_node.AsString());
    }
    if (is_roundtrip || stops.size() <= 1) {
      return stops;
    }
    stops.reserve(stops.size() * 2 - 1);  // end stop is not repeated
    for (size_t stop_idx = stops.size() - 1; stop_idx > 0; --stop_idx) {
      stops.push_back(stops[stop_idx - 1]);
    }
    return stops;
  }

  struct Bus {
    string name;
    vector<string> stops;

    static Bus ParseFrom(const Json::Dict& attrs) {
      return Bus{
          .name = attrs.at("name").AsString(),
          .stops = ParseStops(attrs.at("stops").AsArray(), attrs.at("is_roundtrip").AsBool()),
      };
    }
  };

  using InputQuery = std::variant<Stop, Bus>;

  vector<InputQuery> ReadDescriptions(const vector<Json::Node>& nodes) {
    vector<InputQuery> result;
    result.reserve(nodes.size());

    for (const Json::Node& node : nodes) {
      const auto& node_dict = node.AsMap();
      if (node_dict.at("type").AsString() == "Bus") {
        result.push_back(Bus::ParseFrom(node_dict));
      } else {
        result.push_back(Stop::ParseFrom(node_dict));
      }
    }

    return result;
  }
}

namespace Responses {
  struct Stop {
    set<string> bus_names;
  };

  struct Bus {
    size_t stop_count = 0;
    size_t unique_stop_count = 0;
    double road_route_length = 0.0;
    double geo_route_length = 0.0;
  };
}

class Database {
public:
  using Bus = Responses::Bus;
  using Stop = Responses::Stop;

  explicit Database(vector<Descriptions::InputQuery> data) {
    auto stops_end = std::partition(begin(data), end(data), [](const auto& item) {
      return std::holds_alternative<Descriptions::Stop>(item);
    });

    unordered_map<string, const Descriptions::Stop*> stop_info;
    for (const auto& item : Range{begin(data), stops_end}) {
      const auto& stop = std::get<Descriptions::Stop>(item);
      stop_info[stop.name] = &stop;
      stops.insert({stop.name, {}});
    }

    for (const auto& item : Range{stops_end, end(data)}) {
      const auto& bus = std::get<Descriptions::Bus>(item);

      buses[bus.name] = Bus{
        bus.stops.size(),
        ComputeUniqueItemsCount(AsRange(bus.stops)),
        ComputeRoadRouteLength(bus.stops, stop_info),
        ComputeGeoRouteDistance(bus.stops, stop_info)
      };

      for (const string& stop_name : bus.stops) {
        stops.at(stop_name).bus_names.insert(bus.name);
      }
    }
  }

  const Stop* GetStop(const string& name) const {
    return GetValuePointer(stops, name);
  }

  const Bus* GetBus(const string& name) const {
    return GetValuePointer(buses, name);
  }

private:
  static double ComputeRoadRouteLength(
    const vector<string>& stops,
    const unordered_map<string, const Descriptions::Stop*> stop_info
  ) {
    double result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
      result += Distance(*stop_info.at(stops[i - 1]), *stop_info.at(stops[i]));
    }
    return result;
  }

  static double ComputeGeoRouteDistance(
    const vector<string>& stops,
    const unordered_map<string, const Descriptions::Stop*> stop_info
  ) {
    double result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
      result += Sphere::Distance(
        stop_info.at(stops[i - 1])->position, stop_info.at(stops[i])->position
      );
    }
    return result;
  }

  static double Distance(const Descriptions::Stop& lhs, const Descriptions::Stop& rhs) {
    if (auto it = lhs.distances.find(rhs.name); it != lhs.distances.end()) {
      return it->second;
    } else if (auto it2 = rhs.distances.find(lhs.name); it2 != rhs.distances.end()) {
      return it2->second;
    } else {
      return Sphere::Distance(lhs.position, rhs.position);
    }
  }

  unordered_map<string, Stop> stops;
  unordered_map<string, Bus> buses;
};

namespace Requests {
  struct Stop {
    string name;

    Json::Dict Process(const Database& db) const {
      const auto* stop = db.GetStop(name);
      Json::Dict dict;
      if (!stop) {
        dict["error_message"] = Json::Node(string("not found"));
      } else {
        vector<Json::Node> bus_nodes;
        bus_nodes.reserve(stop->bus_names.size());
        for (const auto& bus_name : stop->bus_names) {
          bus_nodes.emplace_back(bus_name);
        }
        dict["buses"] = Json::Node(move(bus_nodes));
      }
      return dict;
    }
  };

  struct Bus {
    string name;

    Json::Dict Process(const Database& db) const {
      const auto* bus = db.GetBus(name);
      Json::Dict dict;
      if (!bus) {
        dict["error_message"] = Json::Node(string("not found"));
      } else {
        dict = {
            {"stop_count", Json::Node(static_cast<int>(bus->stop_count))},
            {"unique_stop_count", Json::Node(static_cast<int>(bus->unique_stop_count))},
            {"route_length", Json::Node(bus->road_route_length)},
            {"curvature", Json::Node(bus->road_route_length / bus->geo_route_length)},
        };
      }
      return dict;
    }
  };

  variant<Stop, Bus> ReadQuery(const Json::Dict& attrs) {
    if (attrs.at("type").AsString() == "Bus") {
      return Bus{attrs.at("name").AsString()};
    } else {
      return Stop{attrs.at("name").AsString()};
    }
  }
}

vector<Json::Node> ProcessStatRequests(const Database& transport_database, const vector<Json::Node>& stat_requests) {
  vector<Json::Node> responses;
  responses.reserve(stat_requests.size());
  for (const Json::Node& request_node : stat_requests) {
    Json::Dict dict = visit([&transport_database](const auto& request) {
                              return request.Process(transport_database);
                            },
                            Requests::ReadQuery(request_node.AsMap()));
    dict["request_id"] = Json::Node(request_node.AsMap().at("id").AsInt());
    responses.push_back(Json::Node(dict));
  }
  return responses;
}

int main() {
  const auto input_doc = Json::Load(cin);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const Database transport_database{Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray())};

  Json::PrintValue(
    ProcessStatRequests(transport_database, input_map.at("stat_requests").AsArray()),
    cout
  );
  return 0;
}
