#include "Common.h"
#include <unordered_map>
#include <list>
#include <mutex>

using namespace std;

class LruCache : public ICache {
    shared_ptr<IBooksUnpacker> books_unpacker_;
    list<BookPtr> usedBooks_;
    unordered_map<string, list<BookPtr>::iterator> cache_;
    size_t memory_usage = 0;
    size_t max_memory_;
    mutex mutex_;
public:
  LruCache(
      shared_ptr<IBooksUnpacker> books_unpacker,
      const Settings& settings
  ) : books_unpacker_(move(books_unpacker)), max_memory_(settings.max_memory) {}

  BookPtr GetBook(const string& book_name) override {
      unique_lock<mutex> lock(mutex_);
      auto it = cache_.find(book_name);
      if (it != cache_.end()) {
          usedBooks_.splice(usedBooks_.end(), usedBooks_, it->second);
          it->second = prev(usedBooks_.end());
          return *it->second;
      } else {
          lock.unlock(); /// CONCURRENT AREA
          BookPtr book = books_unpacker_->UnpackBook(book_name);
          if (book->GetContent().size() > max_memory_)
              return book;
          lock.lock();  /// END OF CONCURRENT AREA

          memory_usage += book->GetContent().size();
          if (memory_usage > max_memory_) FreeCache();
          usedBooks_.push_back(move(book));
          cache_[book_name] = prev(usedBooks_.end());
          return usedBooks_.back();
      }
  }

protected:
    void FreeCache() {
        while (!usedBooks_.empty() && memory_usage > max_memory_) {
            cache_.erase(usedBooks_.front()->GetName());
            memory_usage -= usedBooks_.front()->GetContent().size();
            usedBooks_.pop_front();
        }
    }

};


unique_ptr<ICache> MakeCache(
    shared_ptr<IBooksUnpacker> books_unpacker,
    const ICache::Settings& settings
) {
  return make_unique<LruCache>(move(books_unpacker), settings);
}
