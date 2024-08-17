#include "builder.h"
#include <fstream>

int main() {

    //std::fstream input("input.txt");

    auto CITY_ROUTER = std::make_unique<Builder>();

    CITY_ROUTER->ParseData();

    CITY_ROUTER->PrintRequest();

    return 0;
}