cmake_minimum_required(VERSION 3.16)
project(scraper_test)

set(CMAKE_CXX_STANDARD 17)
include_directories(../../boost_1_80_0)
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

enable_testing()

add_executable(
        scraper_test
        scraper_test.cpp
        ../query_executor.h
        ../query_executor.cpp
)
target_link_libraries(
        scraper_test
        GTest::gtest_main
        nlohmann_json::nlohmann_json
)
configure_file(data.json data.json COPYONLY)
include(GoogleTest)
gtest_discover_tests(scraper_test)