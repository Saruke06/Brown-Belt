#pragma once

#include <algorithm>

namespace RAII {

template <typename Provider>
class Booking {
private:
    Provider* provider = nullptr;
    int counter = 0;
public:
    Booking(Provider* pr, int c) 
    : provider(pr),
      counter(c)
      {}

    Booking(const Booking&) = delete;
    Booking& operator= (const Booking&) = delete;

    Booking(Booking&& other) 
    : provider(other.provider),
      counter(other.counter)
    {
        other.provider = nullptr;
    }

    Booking& operator= (Booking&& other) {
        std::swap(provider, other.provider);
        std::swap(counter, other.counter);
        return(*this);
    }

    ~Booking() {
        if (provider)
            provider->CancelOrComplete(*this);
    }
};

}