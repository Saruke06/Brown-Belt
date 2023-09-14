#include <iomanip>
#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include <queue>

using namespace std;

struct Booking{
    int64_t time;
    string hotel_name;
    int client_id;
    int room_count;
};

class HotelManager {
public:
    HotelManager() {}

    void Book(const Booking& booking) {

        base.push(booking);

        if (hotel_rooms.count(booking.hotel_name) == 0){
            hotel_rooms[booking.hotel_name] = 0;
        }
        hotel_rooms[booking.hotel_name]+=booking.room_count;

        if (hotel_to_user_and_rooms[booking.hotel_name].count(booking.client_id)==0){
            hotel_to_user_and_rooms[booking.hotel_name][booking.client_id] = 0;
        }
        hotel_to_user_and_rooms[booking.hotel_name][booking.client_id]+=1;

        Pop();
    }

    int Rooms(const string& Hotel_name) const {
        if(hotel_rooms.find(Hotel_name) != hotel_rooms.end()){
            return hotel_rooms.at(Hotel_name);
        } else {
            return 0;
        }
    }

    int Clients(const string& Hotel_name) const {
        if(hotel_to_user_and_rooms.find(Hotel_name) != hotel_to_user_and_rooms.end()){
            return hotel_to_user_and_rooms.at(Hotel_name).size();
        } else {
            return 0;
        }
    }

private:
    queue<Booking> base;
    map<string, map<int, int>> hotel_to_user_and_rooms;
    map<string, int> hotel_rooms;

    void Pop(){
        while (abs(base.back().time - base.front().time) > 86399){
            Booking to_pop = base.front();

            hotel_rooms[to_pop.hotel_name]-=to_pop.room_count;
            hotel_to_user_and_rooms[to_pop.hotel_name][to_pop.client_id]-=1;
            if (hotel_to_user_and_rooms[to_pop.hotel_name][to_pop.client_id]==0){
                hotel_to_user_and_rooms[to_pop.hotel_name].erase(to_pop.client_id);
            }

            base.pop();
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    HotelManager manager;

    int query_count;
    cin >> query_count;

    for (int query_id = 0; query_id < query_count; ++query_id) { // Q
        string query_type;
        cin >> query_type;

        if (query_type == "BOOK") {
            int64_t time;
            string hotel_name;
            int client_id;
            int room_count;
            cin >> time >> hotel_name >> client_id >> room_count;
            manager.Book({time, hotel_name, client_id, room_count});
        } else if (query_type == "CLIENTS") {
            string hotel_name;
            cin >> hotel_name;
            cout << manager.Clients(hotel_name) << "\n";
        } else if(query_type == "ROOMS"){
            string hotel_name;
            cin >> hotel_name;
            cout << manager.Rooms(hotel_name) << "\n";
        }
    }

    return 0;
}
