#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>

#include "range.h"
#include "router.h"
#include "requests.h"
#include "transport_db.h"
#include "json.h"

using namespace std;
using namespace Json;

int main() {
    // TestAllStringParses();
    // TestAllRequests();
    // TestAllTransportDB();
    // TestAllJson();

    const auto input_doc = Json::Load(cin);
    const auto& input_map = input_doc.GetRoot().AsMap();

    const Database transport_database{
        Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
        Descriptions::ParseRouteSettings(input_map.at("routing_settings").AsMap())
    };

    Json::PrintValue(
        ProcessStatRequests(transport_database, input_map.at("stat_requests").AsArray()),
        cout
    );
    return 0;
}