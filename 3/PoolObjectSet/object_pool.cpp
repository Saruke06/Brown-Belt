#include "../../test_runner.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <stdexcept>
#include <set>
#include <memory>

using namespace std;

template <class T>
class ObjectPool {
public:
  T* Allocate();
  T* TryAllocate();

  void Deallocate(T* object);

private:
  // Определяем свой компаратор
  struct Compare {
	  using is_transparent = void; // Сделали компаратор прозрачным

	  // Используем стандартное сравнение ключей
	  bool operator() (const unique_ptr<T>& lhs, const unique_ptr<T>& rhs) const {
		  return lhs < rhs;
	  }

	  // Определяем функции сравненпия ключа и обычного указателя
	  bool operator() (const unique_ptr<T>& lhs, const T* rhs) const {
		  return less<const T*>()(lhs.get(), rhs);
	  }

	  bool operator() (const T* lhs, const unique_ptr<T>& rhs) const {
		  return less<const T*>()(lhs, rhs.get());
	  }

  };
  // Добавьте сюда поля
  queue<unique_ptr<T>> free;
  set<unique_ptr<T>, Compare> allocated;
};

template <typename T>
T* ObjectPool<T>::Allocate() {
	if (free.empty()) {
		free.push(make_unique<T>());
	}
	auto ptr = move(free.front());
	free.pop();
	T* ret = ptr.get();
	allocated.insert(move(ptr));
	return ret;
}

template <typename T>
T* ObjectPool<T>::TryAllocate() {
	if (free.empty()) {
		return nullptr;
	}
	return Allocate();
}

template <typename T>
void ObjectPool<T>::Deallocate(T* object) {
	auto it = allocated.find(object);
	if(it == allocated.end()) {
		throw invalid_argument("");
	}

	free.push(move(allocated.extract(it).value()));
}

void TestObjectPool() {
  ObjectPool<string> pool;

  auto p1 = pool.Allocate();
  auto p2 = pool.Allocate();
  auto p3 = pool.Allocate();

  *p1 = "first";
  *p2 = "second";
  *p3 = "third";

  pool.Deallocate(p2);
  ASSERT_EQUAL(*pool.Allocate(), "second");

  pool.Deallocate(p3);
  pool.Deallocate(p1);
  ASSERT_EQUAL(*pool.Allocate(), "third");
  ASSERT_EQUAL(*pool.Allocate(), "first");

  pool.Deallocate(p1);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestObjectPool);
  return 0;
}
