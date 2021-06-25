#include <coropoll/file_descriptor.hpp>

#include <cassert>

namespace coropoll {

  file_descriptor::file_descriptor(int fd)
    : d_handle()
    , d_fd(fd)
  {}

  bool file_descriptor::await_ready() const {
    return false;
  }

  void file_descriptor::await_suspend(coro_handle handle) {
    // interestingly enough, if this object is already being
    // awaited on by another coroutine, we will lose its handle
    // here. If we resume the first coroutine, we would
    // incorrectly clean up the second's coroutine blocked_on.
    // See the unit tests for more information
    d_handle = handle;

    // update the promise type with our 'fd' because we write this
    // information into the promise, the event loop (through our
    // coroutine type) will be able to figure out what descriptor we
    // are blocked on
    d_handle.promise().fd() = d_fd;
  }

  void file_descriptor::await_resume() {
    // the coroutine will now be resumed, we can clean up
    // the promise's fd.
    d_handle.promise().fd() = std::nullopt;
  }

}
