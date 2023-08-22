#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <sstream>

#include "test_runner.h"

using namespace std;

template <typename Iterator>
class IteratorRange {
public:
  IteratorRange(Iterator begin, Iterator end)
    : first(begin)
    , last(end)
  {
  }

  Iterator begin() const {
    return first;
  }

  Iterator end() const {
    return last;
  }

private:
  Iterator first, last;
};

template <typename Collection>
auto Head(Collection& v, size_t top) {
  return IteratorRange{v.begin(), next(v.begin(), min(top, v.size()))};
}

enum class Gender {
  MALE,
  FEMALE
};

struct Person {
  string name;
  int age, income;
  Gender gender;
};

ostream& operator<< (ostream& os, const Person& p) {
	os << "{" << p.name << ": " << p.age << " " << p.income << " ";
	switch (p.gender) {
	case Gender::MALE:
		 os << 'M';
		 break;
	case Gender::FEMALE:
		os << 'W';
		break;
	default:
		os << "What the fuck is this piece of shit?";
	}
	os << "}";

	return os;
}

vector<Person> ReadPeople(istream& input) {
  int count;
  input >> count;

  vector<Person> result(count);
  for (Person& p : result) {
	char gender;
    input >> p.name >> p.age >> p.income >> gender;
    if (gender == 'M')
    	p.gender = Gender::MALE;
    else
    	p.gender = Gender::FEMALE;
  }

  return result;
}

vector<Person> SortByAge(vector<Person> people) {
	sort(begin(people), end(people), [](const Person& lhs, const Person& rhs) {
	  return lhs.age < rhs.age;
	});
	return move(people);
}

vector<Person> SortByIncome(vector<Person> people) {
	sort(begin(people), end(people), [](const Person& lhs, const Person& rhs) {
	  return lhs.income > rhs.income;
	});
	return move(people);
}

vector<Person> SortGenByName(vector<Person> people) {

    sort(people.begin(), people.end(), [](const Person& lhs, const Person& rhs) {
      return tie(lhs.gender, lhs.name) < tie(rhs.gender, rhs.name);
    });

    return move(people);
}

size_t Age(const vector<Person>& people, int adult_age) {

    auto adult_begin = lower_bound(
      begin(people), end(people), adult_age, [](const Person& lhs, int age) {
        return lhs.age < age;
      }
    );

    return std::distance(adult_begin, end(people));
}

size_t Wealthy(const vector<Person>& people, int count) {

	auto head = Head(people, count);

	size_t total_income = accumulate(
		head.begin(), head.end(), size_t(0), [](size_t cur, const Person& p) {
	    return cur + p.income;
	  }
	);
	return total_income;
}

string PopularName(const vector<Person>& people, char gender) {
	stringstream ss;

	auto first = find_if(begin(people), end(people), [gender](const Person& p) {
		return p.gender == (gender == 'M' ? Gender::MALE : Gender::FEMALE);
	});

    if (first == people.end()) {
      ss << "No people of gender " << gender << '\n';
    } else {
      string most_popular_name = people.begin()->name;
      int count = 1;
      for (auto i = first; i != people.end(); ) {
        auto same_name_end = find_if_not(i, people.end(), [i](const Person& p) {
          return p.name == i->name;
        });
        auto cur_name_count = std::distance(i, same_name_end);
        if (cur_name_count > count) {
          count = cur_name_count;
          most_popular_name = i->name;
        }
        i = same_name_end;
      }
      ss << "Most popular name among people of gender " << gender << " is "
           << most_popular_name << '\n';
    }

    return ss.str();
}

struct Statistics {
	Statistics(string male, string female)
	: popular_name_m_ans(male)
	, popular_name_w_ans(female)
	{
	}

	unordered_map<int, int> age;
	unordered_map<int, int> wealthy;
	string popular_name_m_ans;
	string popular_name_w_ans;
};

int main() {
  vector<Person> people = ReadPeople(cin);

  vector<Person> sorted_by_age = SortByAge(people);
  vector<Person> sorted_by_income = SortByIncome(people);
  vector<Person> sortedMW_by_name = SortGenByName(people);

  Statistics stats(PopularName(sortedMW_by_name, 'M'), PopularName(sortedMW_by_name, 'W'));

  for (string command; cin >> command; ) {
    if (command == "AGE") {
      int adult_age;
      cin >> adult_age;

      if (!stats.age.count(adult_age))
    	  stats.age[adult_age] = Age(sorted_by_age, adult_age);

      cout << "There are " << stats.age[adult_age]
           << " adult people for maturity age " << adult_age << '\n';

    } else if (command == "WEALTHY") {
      int count;
      cin >> count;

      if(!stats.wealthy.count(count))
    	  stats.wealthy[count] = Wealthy(sorted_by_income, count);

      cout << "Top-" << count << " people have total income " << stats.wealthy[count] << '\n';

    } else if (command == "POPULAR_NAME") {
      char gender;
      cin >> gender;

      if (gender == 'M') {
    	  cout << stats.popular_name_m_ans;
      } else {
    	  cout << stats.popular_name_w_ans;
      }

    }
  }
}
