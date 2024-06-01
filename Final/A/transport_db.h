#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

#include "string_parses.h"

struct Stop {
    std::string name;
    double latitude;
    double longitude;
};

struct Bus {
    int number;
    std::vector<Stop> stops;
};



class TransportDatabase {
public:
    TransportDatabase() = default;

    void BuildBase(std::istream& input = std::cin) {
        // To do: READREQUESTS()
        // To do: PROCESSREQUESTS()


    }

private:
std::unordered_map<std::string, Stop> stops;
std::unordered_map<std::string, Bus> buses;

};