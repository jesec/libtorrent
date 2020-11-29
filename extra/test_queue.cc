#include <cstdlib>
#include <iostream>

#include "torrent/utils/priority_queue_default.h"

torrent::utils::priority_queue_default
  queue; //(priority_compare(), priority_equal(), priority_erase());
torrent::utils::priority_item items[100];

int last = 0;

class test {
public:
  test() {}

  void f() {
    std::cout << "Called: " << std::endl;
  }
};

void
print_item(torrent::utils::priority_item* p) {
  std::cout << (p - items) << ' ' << p->time().usec() << std::endl;

  if (p->time().usec() < last) {
    std::cout << "order bork." << std::endl;
    exit(-1);
  }

  last = p->time().usec();
  p->clear_time();

  if (std::rand() % 5) {
    int i = rand() % 100;

    std::cout << "erase " << i << ' ' << items[i].time().usec() << std::endl;
    priority_queue_erase(&queue, items + i);
  }
}

int
main() {
  try {
    test t;

    for (torrent::utils::priority_item *first = items, *last = items + 100;
         first != last;
         ++first) {
      first->set_slot(torrent::utils::mem_fn(&t, &test::f));

      priority_queue_insert(&queue, first, (std::rand() % 50) + 1);
    }

    while (!queue.empty()) {
      torrent::utils::priority_item* i = queue.top();
      queue.pop();

      print_item(i);
    }

  } catch (std::logic_error& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
