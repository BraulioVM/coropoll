#pragma once

#include <coropoll/coroutine.hpp>

namespace coropoll {
  class file_descriptor {
  public:
    // PUBLIC TYPES
    using coro_handle = coroutine::coro_handle;

  private:
    // DATA
    coro_handle d_handle;  // reference to the coroutine frame that is awaiting on us
    int         d_fd;

  public:
    // CREATORS
    explicit file_descriptor(int fd);

    file_descriptor(const file_descriptor&) = delete;
    file_descriptor(file_descriptor&&) = delete;

    // awaitable concept
    bool await_ready() const;
    void await_suspend(coro_handle handle);
    void await_resume();
  };
}
