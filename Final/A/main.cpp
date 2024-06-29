#include <iostream>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>

#include "string_parses.h"
#include "requests.h"
#include "transport_db.h"

#include "test_string_parses.h"
#include "test_requests.h"
#include "test_transport_db.h"

using namespace std;


int main() {
    // TestAllStringParses();
    // TestAllRequests();
    TestAllTransportDB();

    TransportDatabase db;
    const auto modify_requests = ReadRequests();
    ProcessModifyRequests(&db, modify_requests);
    const auto read_requests = ReadRequests();
    const auto responses = ProcessRequests(db, read_requests);
    PrintResponses(responses);

    return 0;
}