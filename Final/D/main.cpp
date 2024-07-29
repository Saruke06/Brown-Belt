#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>

#include "string_parses.h"
#include "requests.h"
#include "transport_db.h"
#include "json.h"

// #include "test_string_parses.h"
// #include "test_requests.h"
// #include "test_transport_db.h"
// #include "test_json.h"

using namespace std;
using namespace Json;


int main() {
    // TestAllStringParses();
    // TestAllRequests();
    // TestAllTransportDB();
    // TestAllJson();

    TransportDatabase db;

    Document input_doc = Load();
    const auto& input_map = input_doc.GetRoot().AsMap();
    const auto modify_requests = ReadRequests(input_map.at("base_requests").AsArray(), true);
    ProcessModifyRequests(&db, modify_requests);
    const auto read_requests = ReadRequests(input_map.at("stat_requests").AsArray(), false);
    const auto responses = ProcessRequests(db, read_requests);
    PrintResponses(responses);

    
    return 0;
}