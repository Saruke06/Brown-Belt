#include <iostream>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_set>

using namespace std;

vector<string> ReadDomains(istream& i_stream = cin) {
    size_t count;
    i_stream >> count;
    vector<string> domains;
    for (size_t i = 0; i < count; ++i) {
        string domain;
        i_stream >> domain;
        domains.emplace_back(move(domain));
    }
    return domains;
}

int main() {
    std::unordered_set<string> forbiden;
    for (const string& domain : ReadDomains(cin)) {
        forbiden.emplace(domain);
    }
    vector<string> domains_to_check = ReadDomains(cin);
    for (const string& domain : domains_to_check) {
        if (forbiden.find(domain) != forbiden.end()) {
            cout << "Bad\n";
            continue;
        }
        string_view subdomain = domain;
        bool is_forbiden = false;
        std::size_t dot_pos = subdomain.find(".");
        while (dot_pos != string::npos) {
            subdomain = subdomain.substr(dot_pos + 1);
            if (forbiden.find(string(subdomain)) != forbiden.end()) {
                is_forbiden = true;
                break;
            }
            dot_pos = subdomain.find(".");
        }
        if (is_forbiden == true)
            cout << "Bad\n";
        else
            cout << "Good\n";
    }
    return 0;
}
