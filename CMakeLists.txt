cmake_minimum_required(VERSION 3.0.0)
project(minmsg VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

if(BUILD_TESTING)
  include(CTest)
  add_subdirectory(test)
endif()

add_library(minmsg INTERFACE)
target_include_directories(minmsg INTERFACE include/) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
