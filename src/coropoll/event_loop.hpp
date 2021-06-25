#pragma once

#include <coropoll/coroutine.hpp>

#include <unordered_map>

namespace coropoll {
  class event_loop {
  private:
    // TYPES
    using fd_t = int;
    using coroutine_map = std::unordered_map<fd_t, coropoll::coroutine>;

    // DATA
    coroutine_map d_coroutines;
    int           d_epoll_fd;
    bool          d_should_quit;

    // PRIVATE MANIPULATORS
    void insert_coroutine(int fd, coroutine);
    void register_in_epoll(int fd);
    void register_all_in_epoll();

    void wakeup_coroutine_blocked_on(int fd);
    
    // PRIVATE ACCESSORS
    bool running() const;
  public:
    // CREATORS
    event_loop();

    // MANIPULATORS
    void schedule(coroutine);
    void run();
    void quit();
  };
}
