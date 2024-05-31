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

using namespace std;




vector<RequestHolder> ReadRequests(istream& input = cin) {
    const size_t request
}

void PrintResponses(const vector<string>& responses, ostream& output = cout) {
    for (const string& response : responses) {
        output << response << '\n';
    }
}

int main() {
    TransportDatabase db;

    db.BuildBase();
    
    const auto requests = ReadRequests();
    const auto responses = ProcessRequests(db, requests);
    PrintResponses(responses);

    return 0;
}