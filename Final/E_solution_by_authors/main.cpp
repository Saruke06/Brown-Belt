#include "builder.h"
#include <fstream>

auto main()->int
{
	std::fstream input("input.txt");
	auto CITY_ROUTER = std::make_unique<Builder>();
#ifdef _DEBUG
	CITY_ROUTER->ParseData(input);
#else
	CITY_ROUTER->ParseData();
#endif // _DEBUG
	CITY_ROUTER->PrintRequest();
	return 0;
}