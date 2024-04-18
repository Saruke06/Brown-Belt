#include "test_runner.h"

#include <cstddef>  // нужно для nullptr_t

using namespace std;

// Реализуйте шаблон класса UniquePtr
template <typename T>
class UniquePtr {
private:
  // ???
  T* ptr;
public:
  UniquePtr() : ptr(nullptr) {};
  UniquePtr(T * ptr) : ptr(ptr) {};
  UniquePtr(const UniquePtr&) = delete;
  UniquePtr(UniquePtr&& other) : ptr(other.Release()) {};

  UniquePtr& operator = (const UniquePtr&) = delete;
  UniquePtr& operator = (nullptr_t) {
    Reset(nullptr);
    return *this;
  }
  UniquePtr& operator = (UniquePtr&& other) {
    if (ptr != other.ptr) {
      Reset(other.Release());
    }
    return *this;
  }

  ~UniquePtr() {
    delete ptr;
  }

  T& operator * () const {
    return *ptr;
  }

  T * operator -> () const {
    return ptr;
  }

  T * Release() {
    T* tmp = ptr;
    ptr = nullptr;
    return tmp;
  }

  void Reset(T * ptr) {
    delete this->ptr;
    this->ptr = ptr;
  }

  void Swap(UniquePtr& other) {
    swap(ptr, other.ptr);
  }

  T * Get() const {
    return ptr;
  }

};


struct Item {
  static int counter;
  int value;
  Item(int v = 0): value(v) {
    ++counter;
  }
  Item(const Item& other): value(other.value) {
    ++counter;
  }
  ~Item() {
    --counter;
  }
};

int Item::counter = 0;


void TestLifetime() {
  Item::counter = 0;
  {
    UniquePtr<Item> ptr(new Item);
    ASSERT_EQUAL(Item::counter, 1);

    ptr.Reset(new Item);
    ASSERT_EQUAL(Item::counter, 1);
  }
  ASSERT_EQUAL(Item::counter, 0);

  {
    UniquePtr<Item> ptr(new Item);
    ASSERT_EQUAL(Item::counter, 1);

    auto rawPtr = ptr.Release();
    ASSERT_EQUAL(Item::counter, 1);

    delete rawPtr;
    ASSERT_EQUAL(Item::counter, 0);
  }
  ASSERT_EQUAL(Item::counter, 0);
}

void TestGetters() {
  UniquePtr<Item> ptr(new Item(42));
  ASSERT_EQUAL(ptr.Get()->value, 42);
  ASSERT_EQUAL((*ptr).value, 42);
  ASSERT_EQUAL(ptr->value, 42);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestLifetime);
  RUN_TEST(tr, TestGetters);
}
