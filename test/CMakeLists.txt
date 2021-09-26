cmake_minimum_required(VERSION 3.0.0)
project(minmsg-test VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

add_executable(minmsg-test test.cpp)
add_test(minmsg-test minmsg-test)