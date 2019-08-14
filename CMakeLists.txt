cmake_minimum_required(VERSION 2.8)
project(rambler_banners)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIR})
set(HEADERS
        ${INCLUDE_DIR}/hash_back_inserter.h
        ${INCLUDE_DIR}/simple_thread_pool.h
        ${INCLUDE_DIR}//tuple_utils.h include/banner_traits.h include/banner.h include/banner_ex.h include/filter.h include/lyciator.h)

add_subdirectory(test)
add_executable(rambler_banners_${CMAKE_BUILD_TYPE} src/main.cpp ${HEADERS})