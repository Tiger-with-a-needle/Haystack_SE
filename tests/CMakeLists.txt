include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${haystack_search_engine_SOURCE_DIR}/include)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW) # to avoid DOWNLOAD_EXTRACT_TIMESTAMP option message
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL
    https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip # v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(PROJECT_SOURCES

    ../src/converters.cpp
    ../src/exceptions.cpp
    ../src/servers.cpp
    ../src/telemetry.cpp
    # ...
)

add_executable(haystack_engine_tests tests.cpp ${PROJECT_SOURCES})

target_link_libraries(haystack_engine_tests GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(haystack_engine_tests)