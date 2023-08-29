#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;


struct Email {
  string from;
  string to;
  string body;
};


class Worker {
public:
  virtual ~Worker() = default;
  virtual void Process(unique_ptr<Email> email) = 0;
  virtual void Run() {
    // только первому worker-у в пайплайне нужно это имплементировать
    throw logic_error("Unimplemented");
  }

protected:
  // реализации должны вызывать PassOn, чтобы передать объект дальше
  // по цепочке обработчиков
  void PassOn(unique_ptr<Email> email) const {
	  if (worker != nullptr)
		  worker->Process(move(email));
  }

public:
  void SetNext(unique_ptr<Worker> next) {
	  worker = move(next);
  }
protected:
  unique_ptr<Worker> worker = nullptr;
};


class Reader : public Worker {
public:
  // реализуйте класс
  Reader(istream& is)
  : input(is)
  {}

  void Process(unique_ptr<Email> email) override {
	  PassOn(move(email));
  }

  void Run() {
	  for(string from, to, body;
		  getline(input, from) && getline(input, to) && getline(input, body); ) {
		  PassOn(make_unique<Email>(Email{from, to, body}));
	  }
  }

private:
  istream& input;
};


class Filter : public Worker {
public:
  using Function = function<bool(const Email&)>;
  Filter(Function f_)
  :  f(f_)
  {}

  void Process(unique_ptr<Email> email) override {
	  if (f(*email)) {
		  PassOn(move(email));
	  }
  }
public:
  // реализуйте класс
  Function f;
};


class Copier : public Worker {
public:
  // реализуйте класс
	Copier(string address_)
	: address(address_)
	{}

	void Process(unique_ptr<Email> email) override {
		if (email->to != address) {
			unique_ptr<Email> copy = make_unique<Email>(Email{email->from, address, email->body});
			PassOn(move(email));
			PassOn(move(copy));
		} else {
			PassOn(move(email));
		}
	}

private:
	string address;
};


class Sender : public Worker {
public:
  // реализуйте класс
  Sender(ostream& os_)
  : os(os_)
  {}

  void Process(unique_ptr<Email> email) override {
	  os << email->from << '\n'
	     << email->to << '\n'
		 << email->body << '\n';
	  PassOn(move(email));
  }
private:
  ostream& os;
};


// реализуйте класс
class PipelineBuilder {
private:
	vector<unique_ptr<Worker>> pointers;

public:
  // добавляет в качестве первого обработчика Reader
  explicit PipelineBuilder(istream& in) {
	  pointers.push_back(make_unique<Reader>(in));
  }

  // добавляет новый обработчик Filter
  PipelineBuilder& FilterBy(Filter::Function filter) {
	  pointers.push_back(make_unique<Filter>(filter));
	  return *this;
  }

  // добавляет новый обработчик Copier
  PipelineBuilder& CopyTo(string recipient) {
	  pointers.push_back(make_unique<Copier>(recipient));
	  return *this;
  }

  // добавляет новый обработчик Sender
  PipelineBuilder& Send(ostream& out) {
	  pointers.push_back(make_unique<Sender>(out));
	  return *this;
  }

  // возвращает готовую цепочку обработчиков
  unique_ptr<Worker> Build() {
	  while(pointers.size() > 1) {
		  auto ptr = move(pointers.back());
		  pointers.pop_back();
		  pointers.back()->SetNext(move(ptr));
	  }
	  return move(pointers.back());
  }
};


void TestSanity() {
  string input = (
    "erich@example.com\n"
    "richard@example.com\n"
    "Hello there\n"

    "erich@example.com\n"
    "ralph@example.com\n"
    "Are you sure you pressed the right button?\n"

    "ralph@example.com\n"
    "erich@example.com\n"
    "I do not make mistakes of that kind\n"
  );
  istringstream inStream(input);
  ostringstream outStream;

  PipelineBuilder builder(inStream);
  builder.FilterBy([](const Email& email) {
    return email.from == "erich@example.com";
  });
  builder.CopyTo("richard@example.com");
  builder.Send(outStream);
  auto pipeline = builder.Build();

  pipeline->Run();

  string expectedOutput = (
    "erich@example.com\n"
    "richard@example.com\n"
    "Hello there\n"

    "erich@example.com\n"
    "ralph@example.com\n"
    "Are you sure you pressed the right button?\n"

    "erich@example.com\n"
    "richard@example.com\n"
    "Are you sure you pressed the right button?\n"
  );

  ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestSanity);
  return 0;
}
