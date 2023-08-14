#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

using namespace std;

struct Record {
  string id;
  string title;
  string user;
  int timestamp;
  int karma;
};

bool operator== (const Record& lhs, const Record& rhs) {
	return lhs.id == rhs.id &&
		   lhs.title == rhs.title &&
		   lhs.user == rhs.user &&
		   lhs.timestamp == rhs.timestamp &&
		   lhs.karma == rhs.karma;
}

// Реализуйте этот класс
class Database {
private:
	struct data {
		Record record;
		multimap<int, const Record*>::iterator time_it;
		multimap<int, const Record*>::iterator karma_it;
		multimap<string, const Record*>::iterator user_it;
	};
  unordered_map<string, data> id_record;
  multimap<int, const Record*> time_rec;
  multimap<int, const Record*> karma_rec;
  multimap<string, const Record*> user_rec;
public:
  bool Put(const Record& record) {
	  auto [it, flag] = id_record.insert(make_pair(record.id, data{ record, {}, {}, {} } ));
	  if (!flag)
		  return false;

	  data& rec = it->second;
	  const Record* ptr = &rec.record;
	  rec.time_it = time_rec.insert(make_pair(ptr->timestamp, ptr));
	  rec.karma_it = karma_rec.insert(make_pair(ptr->karma, ptr));
	  rec.user_it = user_rec.insert(make_pair(ptr->user, ptr));
	  return true;
  }

  const Record* GetById(const string& id) const {
	  auto it = id_record.find(id);
	  if (it == id_record.end()) {
		  return nullptr;
	  } else {
		  return &(it->second.record);
	  }
  }

  bool Erase(const string& id) {
	  auto it = id_record.find(id);
	  bool f = false;
	  if (it != id_record.end()) {
		  f = true;
		  time_rec.erase(it->second.time_it);
		  karma_rec.erase(it->second.karma_it);
		  user_rec.erase(it->second.user_it);
		  id_record.erase(it);
	  }
	  return f;
  }

  template <typename Callback>
  void RangeByTimestamp(int low, int high, Callback callback) const {
	  auto it_begin = time_rec.lower_bound(low);
	  auto it_end = time_rec.upper_bound(high);
	  for (auto it = it_begin; it != it_end; ++it) {
		  if(!callback(*(it->second))) {
			  break;
		  }
	  }
  }

  template <typename Callback>
  void RangeByKarma(int low, int high, Callback callback) const {
	  auto it_begin = karma_rec.lower_bound(low);
	  auto it_end = karma_rec.upper_bound(high);
	  for (auto it = it_begin; it != it_end; ++it) {
		  if(!callback(*(it->second))) {
			  break;
		  }
	  }
  }

  template <typename Callback>
  void AllByUser(const string& user, Callback callback) const {
	  auto it_begin = user_rec.lower_bound(user);
	  auto it_end = user_rec.upper_bound(user);
	  for (auto it = it_begin; it != it_end; ++it) {
		  if(!callback(*(it->second))) {
			  break;
		  }
	  }
  }

};

void TestRangeBoundaries() {
  const int good_karma = 1000;
  const int bad_karma = -10;

  Database db;
  db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
  db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

  int count = 0;
  db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
    ++count;
    return true;
  });

  ASSERT_EQUAL(2, count);
}

void TestSameUser() {
  Database db;
  db.Put({"id1", "Don't sell", "master", 1536107260, 1000});
  db.Put({"id2", "Rethink life", "master", 1536107260, 2000});

  int count = 0;
  db.AllByUser("master", [&count](const Record&) {
    ++count;
    return true;
  });

  ASSERT_EQUAL(2, count);
}

void TestReplacement() {
  const string final_body = "Feeling sad";

  Database db;
  db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
  db.Erase("id");
  db.Put({"id", final_body, "not-master", 1536107260, -10});

  auto record = db.GetById("id");
  ASSERT(record != nullptr);
  ASSERT_EQUAL(final_body, record->title);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestRangeBoundaries);
  RUN_TEST(tr, TestSameUser);
  RUN_TEST(tr, TestReplacement);
  return 0;
}
