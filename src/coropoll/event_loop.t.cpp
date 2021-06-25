#include <coropoll/event_loop.hpp>
#include <coropoll/coroutine.hpp>
#include <coropoll/util.hpp>
#include <coropoll/file_descriptor.hpp>

#include <gtest/gtest.h>

#include <cstdio>
#include <unistd.h>

#include <array>
#include <thread>

namespace coropoll {
  TEST(event_loop_tests, breathing_test)
  {
    event_loop ev;
  }

  
  class test_pipe {
  private:
    // DATA
    std::array<int, 2> d_fds;
    std::jthread d_thread;

  public:
    // CREATORS
    test_pipe()
      : d_fds{}
      , d_thread()
    {
      util::call_or_abort("pipe", ::pipe, d_fds.data());
    }

    test_pipe(const test_pipe&) = delete;
    test_pipe(test_pipe&&) = delete;

    ~test_pipe() {
      util::call_or_abort("close", ::close, d_fds[0]);
      util::call_or_abort("close", ::close, d_fds[1]);
    }

    // MANIPULATORS
    void wakeup_in(int seconds) {
      d_thread = std::jthread([this, seconds](){
	sleep(seconds);
	util::call_or_abort("test_pipe_write", ::write, d_fds[1], "a", 1);
      });
    }

    void wakeup_now() {
      util::call_or_abort("test_pipe_write_now", ::write, d_fds[1], "a", 1);
    }

    // ACCESORS
    int read_fd() const {
      return d_fds[0];
    }
  };
  
  TEST(event_loop_tests, coroutine_gets_woken_up)
  {
    // GIVEN
    event_loop ev;
    test_pipe pipe;
    auto lambda = [&pipe, &ev]() -> coroutine {
      co_await file_descriptor{pipe.read_fd()};
      ev.quit();      
    };
    ev.schedule(lambda());

    // WHEN
    pipe.wakeup_in(1);
    ev.run();

    // THEN: test execution finishes
  }

  TEST(event_loop_tests, coroutines_can_get_woken_up_in_opposite_order)
  {
    // GIVEN
    event_loop ev;
    test_pipe pipe1, pipe2, pipe3;
    int finished_coroutines = 0;
    auto lambda = [&ev, &finished_coroutines](const int fd) -> coroutine {
      co_await file_descriptor{fd};
      if (++finished_coroutines == 3) {
	ev.quit();
      }
    };

    ev.schedule(lambda(pipe1.read_fd()));
    ev.schedule(lambda(pipe2.read_fd()));
    ev.schedule(lambda(pipe3.read_fd()));

    // WHEN
    pipe3.wakeup_in(1);
    pipe2.wakeup_in(2);
    pipe1.wakeup_in(3);

    // THEN: event loop runs and eventually finishes
    ev.run();
  }

  TEST(event_loop_tests, coroutines_can_wake_up_one_another)
  {
    // GIVEN
    event_loop ev;
    test_pipe pipe1, pipe2, pipe3;

    auto lam1 = [&]() -> coroutine {
      co_await file_descriptor{pipe1.read_fd()};
      pipe2.wakeup_now();
    };

    auto lam2 = [&]() -> coroutine {
      co_await file_descriptor{pipe2.read_fd()};
      pipe3.wakeup_now();
    };

    auto lam3 = [&]() -> coroutine {
      co_await file_descriptor{pipe3.read_fd()};
      ev.quit();
    };

    ev.schedule(lam1());
    ev.schedule(lam2());
    ev.schedule(lam3());

    // WHEN
    pipe1.wakeup_in(1);

    // THEN
    ev.run();
  }


  TEST(event_loop_tests, can_add_coroutine_while_loop_is_running)
  {
    // GIVEN
    event_loop ev;
    test_pipe pipe1, pipe2;

    auto scheduler_coro = [&]() -> coroutine {
      auto quitter_coro = [&]() -> coroutine {
	co_await file_descriptor{pipe2.read_fd()};
	ev.quit();
      };

      co_await file_descriptor{pipe1.read_fd()};
      ev.schedule(quitter_coro());
    };

    ev.schedule(scheduler_coro());

    // WHEN
    pipe2.wakeup_now();
    pipe1.wakeup_in(1);
    
    // THEN
    ev.run();    
  }
}
  
