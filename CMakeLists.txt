cmake_minimum_required(VERSION 3.31)
project(verifycxx VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

add_executable(verifycxx_test verifycxx/quick_test.cpp)

target_link_libraries(verifycxx_test PRIVATE
        GTest::gtest
        GTest::gtest_main
        benchmark::benchmark
        benchmark::benchmark_main
        Threads::Threads
)
target_include_directories(verifycxx_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/verifycxx
)

enable_testing()
add_test(NAME verifycxx_test COMMAND verifycxx_test)

include(GoogleTest)
## gtest_discover_tests(verifycxx_test)