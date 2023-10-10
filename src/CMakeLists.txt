include_directories(${haystack_search_engine_SOURCE_DIR}/include)

set(PROJECT_SOURCES

    converters.cpp
    exceptions.cpp
    servers.cpp
    telemetry.cpp
    # ...
)

add_executable(haystack_search_engine main.cpp ${PROJECT_SOURCES})