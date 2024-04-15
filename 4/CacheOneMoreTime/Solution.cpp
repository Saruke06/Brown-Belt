#include "Common.h"

#include <vector>
#include <list>
#include <mutex>
#include <unordered_map>

using namespace std;

class LruCache : public ICache {
  shared_ptr<IBooksUnpacker> books_unpacker_;
  list<BookPtr> books_;
  unordered_map<string, list<BookPtr>::iterator> cache_;
  size_t max_memory_;
  size_t memory_usage_ = 0;
  mutable mutex m;

public:
  LruCache(
      shared_ptr<IBooksUnpacker> books_unpacker,
      const Settings& settings
  ) : books_unpacker_(move(books_unpacker)), max_memory_(settings.max_memory)
  {
    // реализуйте метод
  }

  BookPtr GetBook(const string& book_name) override {
    // реализуйте метод
    unique_lock<mutex> lock(m);
    auto it = cache_.find(book_name);
    if (it != cache_.end()) {
      // если книга есть в кэше
      books_.splice(books_.end(), books_, it->second);
      it->second = prev(books_.end());
      return *it->second;
    } else {
      // если книги нет в кэше
      lock.unlock(); // КОНКУРЕНТНАЯ ОБЛАСТЬ
      BookPtr book = books_unpacker_->UnpackBook(book_name);
      if (book->GetContent().size() > max_memory_)
        return book;
      lock.lock(); // КОНЕЦ КОНКУРЕНТНОЙ ОБЛАСТИ
      memory_usage_ += book->GetContent().size();
      if (memory_usage_ > max_memory_)
        FreeCache();
      books_.push_back(move(book));
      cache_[book_name] = prev(books_.end());
      return books_.back();
    }
  }

protected:
  void FreeCache() {
    while (!books_.empty() && memory_usage_ > max_memory_) {
      cache_.erase(books_.front()->GetName());
      memory_usage_ -= books_.front()->GetContent().size();
      books_.pop_front();
    }
  }
};


unique_ptr<ICache> MakeCache(
    shared_ptr<IBooksUnpacker> books_unpacker,
    const ICache::Settings& settings
) {
  // реализуйте функцию
  return make_unique<LruCache>(move(books_unpacker), settings);
}
