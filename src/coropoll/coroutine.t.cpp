#include <coropoll/coroutine.hpp>

#include <coroutine>
#include <iostream>
#include <gtest/gtest.h>

namespace coropoll {

  TEST(coroutine_tests, coroutine_can_be_created)
  {
    // compiles
    auto coro = ([]() -> coroutine {
      co_return;
    })();
  }

  TEST(coroutine_tests, blocked_on_may_return_none)
  {
    // GIVEN
    auto coro = ([]() -> coroutine {
      co_return;
    })();

    // THEN
    ASSERT_FALSE(coro.blocked_on());
  }

  TEST(coroutine_tests, blocked_on_may_return_value)
  {
    // TODO: Not sure how to test this without creating
    // a custom awaitable that sets the fd in the promise
    // type. I can also test this by awaiting on a file_descriptor,
    // but do not want this test to depend on the other
    // component.
  }

  TEST(coroutine_tests, coroutine_can_be_resumed)
  {
    // GIVEN
    bool witnessed = false;
    auto lambda = [&witnessed]() -> coroutine {
      co_await std::suspend_always{};
      witnessed = true;
    };
    auto coro = lambda();
    ASSERT_FALSE(witnessed);

    // WHEN
    coro.resume();

    // THEN
    ASSERT_TRUE(witnessed);
  }
}

