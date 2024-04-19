#pragma once

namespace RAII {

template <typename Provider>
class Booking {
private:
    Provider* provider;
    int counter = 0;
public:
    Booking();

    ~Booking();
};

}