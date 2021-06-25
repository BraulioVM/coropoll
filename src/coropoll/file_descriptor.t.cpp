#include <coropoll/file_descriptor.hpp>
#include <coropoll/coroutine.hpp>

#include <gtest/gtest.h>

namespace coropoll {

  TEST(file_descriptor_tests, can_await_on_it)
  {
    // GIVEN
    auto coro = ([]() -> coroutine {
      co_await file_descriptor{100};
    })();
  }

  TEST(file_descriptor_tests, blocked_on_returns_right_descriptor)
  {
    // GIVEN
    const int fd = 1001;
    auto lambda = [fd]() -> coroutine {
      co_await file_descriptor{fd};
    };
    auto coro = lambda();

    // THEN
    ASSERT_EQ(coro.blocked_on(), fd);
  }


  TEST(file_descriptor_tests, blocked_on_gets_first_result)
  {
    // GIVEN
    const int fd1 = 10030, fd2 = -1;
    auto lambda = [fd1, fd2]() -> coroutine {
      co_await file_descriptor{fd1};
      co_await file_descriptor{fd2};
    };
    auto coro = lambda();

    // THEN
    ASSERT_EQ(coro.blocked_on(), fd1);
  }
  
  TEST(file_descriptor_tests, blocked_on_gets_updated_when_resumed)
  {
    // GIVEN
    const int fd1 = 10030, fd2 = 31337;
    auto lambda = [fd1, fd2]() -> coroutine {
      co_await file_descriptor{fd1};
      co_await file_descriptor{fd2};
    };
    auto coro = lambda();

    // WHEN
    coro.resume();

    // THEN
    ASSERT_EQ(coro.blocked_on(), fd2);
  }

  TEST(file_descriptor_tests, blocked_on_gets_cleared_when_finished)
  {
    // GIVEN
    auto coro = ([]() -> coroutine {
      co_await file_descriptor{31337};
    })();

    // WHEN
    coro.resume();

    // THEN
    ASSERT_EQ(coro.blocked_on(), std::nullopt);
    
  }

  TEST(file_descriptor_tests, same_descriptor_can_be_used_in_two_coroutines)
  {
    // GIVEN
    file_descriptor fd{200};
    auto l = [&fd]() -> coroutine {
      co_await fd;
    };
    auto coro1 = l();
    auto coro2 = l();

    // THEN
    EXPECT_EQ(coro1.blocked_on(), 200);
    EXPECT_EQ(coro2.blocked_on(), 200);

    // Also: we should probably forbid having two different coroutines
    // awaiting on the same `file_descriptor`. See:
    coro1.resume();

    EXPECT_EQ(coro1.blocked_on(), 200);  // it should be the other way around!!
    EXPECT_EQ(coro2.blocked_on(), std::nullopt);
  }
}
