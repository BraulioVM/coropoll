#include <coropoll/event_loop.hpp>
#include <coropoll/util.hpp>

#include <algorithm>
#include <cassert>
#include <utility>

#include <sys/epoll.h>

namespace coropoll {

  namespace {
    
  }

  void event_loop::insert_coroutine(int fd, coroutine coro) {
    auto to_be_inserted = std::make_pair(fd, std::move(coro));
    d_coroutines.insert(std::move(to_be_inserted));
  }

  void event_loop::register_in_epoll(int fd) {
    assert(running());

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;

    util::call_or_abort("epoll_ctl",
			epoll_ctl,
			d_epoll_fd,
			EPOLL_CTL_ADD,
			fd,
			&event);
  }

  void event_loop::register_all_in_epoll() {
    assert(running());

    std::for_each(d_coroutines.begin(), d_coroutines.end(), [&](const auto& pair) {
      const auto& [fd, _] = pair;
      register_in_epoll(fd);
    });
  }

  void event_loop::wakeup_coroutine_blocked_on(int fd) {
    assert(running());
    auto it = d_coroutines.find(fd);

    // a fd is in our epoll set if and only if it is a key in our
    // unordered_map. If that's not the case, there's a bug in the
    // implementation
    assert(it != d_coroutines.end());

    coroutine coro = std::move(it->second); // extract the coroutine from the map
    d_coroutines.erase(it);

    util::call_or_abort("epollctl delete",
			epoll_ctl,
			d_epoll_fd,
			EPOLL_CTL_DEL,
			fd,
			nullptr);

    coro.resume();

    if (coro.blocked_on()) {
      // blocked on a fd again?
      int new_fd = *coro.blocked_on();
      insert_coroutine(new_fd, std::move(coro));
      register_in_epoll(new_fd);
    }
  }

  bool event_loop::running() const {
    return d_epoll_fd != -1;
  }

  event_loop::event_loop()
    : d_coroutines()
    , d_epoll_fd{-1}
    , d_should_quit{false}
  {}

  void event_loop::schedule(coroutine coro) {
    std::optional<int> ofd = coro.blocked_on();

    if (!ofd) {
      // no idea what this coroutine is blocked on or even
      // if it's blocked
      return;
    }

    int fd = *ofd;

    // add it to the map
    insert_coroutine(fd, std::move(coro));

    if (running()) {
      register_in_epoll(fd);
    }
  }

  void event_loop::run() {
    d_epoll_fd = util::call_or_abort("epoll_create1",
				     epoll_create1,
				     0);

    register_all_in_epoll();

    d_should_quit = false;

    while (!d_should_quit) {
      // *the* event loop
      std::array<epoll_event, 10> events;
      int nfds = util::call_or_abort("epoll_wait",
				     epoll_wait,
				     d_epoll_fd,
				     events.data(),
				     events.size(),
				     -1);
      
      std::for_each(events.begin(), events.begin() + nfds, [&](const epoll_event &ev) {
	int coro_fd = ev.data.fd;
	wakeup_coroutine_blocked_on(coro_fd);
      });
    }

    util::call_or_abort("close", close, d_epoll_fd);
    d_epoll_fd = -1;
  }

  void event_loop::quit() {
    d_should_quit = true;
  }
}
