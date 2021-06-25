#pragma once

#include <utility>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

namespace coropoll::util {
  template<typename Syscall, typename... Arguments>
  int call_or_abort(const char *message,
		    Syscall syscall,
		    Arguments&&... arguments);




  // implementation
  template<typename Syscall, typename... Arguments>
  int call_or_abort(const char *message,
		    Syscall syscall,
		    Arguments&&... arguments) {
    int result = syscall(std::forward<Arguments>(arguments)...);

    if (result == -1) {
      perror(message);
      std::abort();
    }

    return result;
  }

  
}
