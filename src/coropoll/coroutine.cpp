#include <coropoll/coroutine.hpp>

#include <cassert>
#include <utility>

namespace coropoll {
  coroutine::coroutine(coro_handle handle)
    : d_handle(handle)
  {}

  coroutine::coroutine(coroutine&& other)
    : d_handle(std::exchange(other.d_handle, nullptr))
  {}

  coroutine::~coroutine() {
    if (d_handle) {
      d_handle.destroy();
    }
  }


  // MANIPULATORS
  void coroutine::resume() {
    assert(d_handle);
    assert(!d_handle.done());

    d_handle.resume();
  }


  // ACCESSORS
  std::optional<int> coroutine::blocked_on() const {
    assert(d_handle);
    return d_handle.promise().fd();
  }

  // promise type
  using promise_t = coroutine::promise_type;
  coroutine promise_t::get_return_object() {
    return coroutine{coro_handle::from_promise(*this)};
  }

  std::suspend_never promise_t::initial_suspend() {
    return std::suspend_never{};
  }

  std::suspend_always promise_t::final_suspend() {
    return std::suspend_always{};
  }

  void promise_t::unhandled_exception() {
    std::terminate();
  }

  void promise_t::return_void() {}

}
