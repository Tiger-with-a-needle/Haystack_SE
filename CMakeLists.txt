cmake_minimum_required(VERSION 3.14)
project(haystack_search_engine VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(src)

# The following option turns tests build ON/OFF
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()