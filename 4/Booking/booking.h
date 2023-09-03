#pragma once

namespace RAII {

template <typename Provider>
class Booking {
private:
	Provider* provider = nullptr;
	int counter = 0;
public:
	Booking(Provider* pr, int c) {
		if (c) {
			provider = pr;
			counter = c;
		}
	}

	Booking(Booking&& other) {
		if (provider)
			provider->CancelOrComplete(*this);
		provider = other.provider;
		other.provider = nullptr;
		counter = other.counter;
	}

	Booking& operator=(Booking&& other) {
		if (provider)
			provider->CancelOrComplete(*this);
		provider = other.provider;
		other.provider = nullptr;
		counter = other.counter;
		return *this;
	}

	~Booking() {
		if (provider)
		    provider->CancelOrComplete(*this);
	}
};

}
