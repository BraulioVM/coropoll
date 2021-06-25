cmake_minimum_required(VERSION 3.15)

project(epoll)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-fcoroutines")
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(googletest)
enable_testing()

# coropoll library
add_library(coropoll STATIC
  src/coropoll/coroutine.cpp
  src/coropoll/file_descriptor.cpp
  src/coropoll/event_loop.cpp
  src/coropoll/util.cpp)
target_include_directories(coropoll PUBLIC src)


# coropoll tests
add_executable(coropoll-tests
  src/coropoll/coroutine.t.cpp
  src/coropoll/file_descriptor.t.cpp
  src/coropoll/event_loop.t.cpp)
target_link_libraries(coropoll-tests PRIVATE
  coropoll
  gtest
  gtest_main)
include(GoogleTest)
gtest_discover_tests(coropoll-tests)
