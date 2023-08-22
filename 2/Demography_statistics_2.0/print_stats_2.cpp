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

pair<string, string> PopularName(vector<Person> people) {
	pair<string, string> result = {"", ""};

	auto male_end = partition(begin(people), end(people), [](Person& p) {
        return p.gender == Gender::MALE;
    });

	IteratorRange male_range {
		begin(people),
		male_end
	};
	IteratorRange female_range {
		male_end,
		end(people)
	};


	if (male_range.begin() != male_range.end()) {
		unordered_map<string, int> frequency_male;

		for (const Person& person : male_range) {
			frequency_male[person.name]++;
		}

		int maxCount = 0;
		string mostFrequentString = frequency_male.begin()->first;

		for (const auto& entry : frequency_male) {
			if (entry.second > maxCount ||
				(entry.second == maxCount && entry.first < mostFrequentString)) {
			    maxCount = entry.second;
			    mostFrequentString = entry.first;
			}
		}
		result.first = mostFrequentString;
	}

	if (female_range.begin() != female_range.end()) {
		unordered_map<string, int> frequency_female;

		for (const Person& person : female_range) {
			frequency_female[person.name]++;
		}

		int maxCount = 0;
		string mostFrequentString = frequency_female.begin()->first;

		for (const auto& entry : frequency_female) {
			if (entry.second > maxCount ||
				(entry.second == maxCount && entry.first < mostFrequentString)) {
			    maxCount = entry.second;
			    mostFrequentString = entry.first;
			}
		}
		result.second = mostFrequentString;
	}

    return result;
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

  pair<string, string> popularNames = PopularName(people);

  Statistics stats(popularNames.first, popularNames.second);

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
    	  if (stats.popular_name_m_ans != "") {
    		  cout << "Most popular name among people of gender " << gender << " is "
    		       << stats.popular_name_m_ans << '\n';
    		  continue;
    	  }
      } else {
    	  if (stats.popular_name_w_ans != "") {
    		  cout << "Most popular name among people of gender " << gender << " is "
    		       << stats.popular_name_w_ans << '\n';
    		  continue;
    	  }
      }
      cout << "No people of gender " << gender << '\n';

    }
  }
}
