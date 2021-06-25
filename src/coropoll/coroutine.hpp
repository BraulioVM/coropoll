#pragma once

#include <coroutine>
#include <optional>

namespace coropoll {
  class coroutine {
  public:
    // PUBLIC TYPES
    class promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

  private:
    // DATA
    coro_handle d_handle;

  public:
    // CREATORS
    explicit coroutine(coro_handle handle);
    coroutine(const coroutine&) = delete;
    coroutine(coroutine &&);

    coroutine& operator=(const coroutine&) = delete;
    coroutine& operator=(coroutine&&) = delete;

    ~coroutine();

    // MANIPULATORS
    void resume();
      // Resume the coroutine. Behavior is undefined if
      // the coroutine is done.

    // ACCESSORS
    std::optional<int> blocked_on() const;
      // If the coroutine is blocked waiting for a file descriptor,
      // return it. Otherwise, return std::nullopt.
  };

  class coroutine::promise_type {
  private:
    // DATA

    // The file descriptor the coroutine is waiting for,
    // if waiting for one. `std::nullopt` otherwise.
    std::optional<int> d_fd;

  public:
    // MANIPULATORS
    std::optional<int>& fd() { return d_fd; }

    // ACCESSORS
    const std::optional<int>& fd() const { return d_fd; }

    // coroutine concept
    coroutine get_return_object();
    std::suspend_never initial_suspend();
    std::suspend_always final_suspend();
    void unhandled_exception();
    void return_void();
  }; 
}
