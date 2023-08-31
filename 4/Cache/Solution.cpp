#include "Common.h"

#include <mutex>
#include <queue>
#include <list>

using namespace std;

class LruCache : public ICache {
private:
	mutable mutex m;
	list<BookPtr> cache;
	Settings settings;
	shared_ptr<IBooksUnpacker> unpacker;
	size_t current_memory = 0;
public:
  LruCache(
      shared_ptr<IBooksUnpacker> books_unpacker,
      const Settings& settings_
  ) {
    // реализуйте метод
	  settings = settings_;
	  unpacker = books_unpacker;
  }

  BookPtr GetBook(const string& book_name) override {
    // реализуйте метод
	  lock_guard<mutex> g(m);

	  // Ищем и находим
	  for (auto it = cache.begin(); it != cache.end(); ++it) {
		  const string& cur_name = (*it)->GetName();
		  if (cur_name == book_name) {
			  cache.splice(cache.begin(), cache, it);
			  return cache.front();
		  }
	  }
	  // Не нашли
	  BookPtr book = BookPtr(unpacker->UnpackBook(book_name));
	  const string& cur_content = book->GetContent();

	  // Если общий размер книг превышает settings.max_memory,
	  // то удаляются книги с наименьшим рангом, пока необходимо
	  while (!cache.empty() &&
			 (cur_content.size() + current_memory) > settings.max_memory)
	  {
		  current_memory -= cache.back()->GetContent().size();
		  cache.pop_back();
	  }

	  // Проверяем, вместится ли книга в кэш и добавляем, если да
	  if (cur_content.size() + current_memory <= settings.max_memory) {
		  cache.push_front(book);
		  current_memory += cur_content.size();
	  }

	  return book;
  }
};


unique_ptr<ICache> MakeCache(
    shared_ptr<IBooksUnpacker> books_unpacker,
    const ICache::Settings& settings
) {
  // реализуйте функцию
	return make_unique<LruCache>(books_unpacker, settings);
}
