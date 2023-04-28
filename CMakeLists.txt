cmake_minimum_required(VERSION 3.10)
project(rambler_banners)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

include_directories(${INCLUDE_DIR})
set(HEADERS
        ${INCLUDE_DIR}/hash_back_inserter.h
        ${INCLUDE_DIR}/simple_thread_pool.h
        ${INCLUDE_DIR}/tuple_utils.h
        ${INCLUDE_DIR}/banner_traits.h
        ${INCLUDE_DIR}/banner.h
        ${INCLUDE_DIR}/banner_ex.h
        ${INCLUDE_DIR}/filter.h
        ${INCLUDE_DIR}/lyciator.h
        )

add_library(coverage_config INTERFACE)
option(CODE_COVERAGE "Enable coverage reporting" OFF)
if (CODE_COVERAGE)
    target_compile_options(coverage_config INTERFACE
            -O0
            -g
            --coverage
            )
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
        target_link_options(coverage_config INTERFACE --coverage)
    else()
        target_link_libraries(coverage_config INTERFACE --coverage)
    endif()
endif(CODE_COVERAGE)

add_subdirectory(src)

option (BUILD_TESTING "Build the testing tree." ON)
if (BUILD_TESTING AND (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    enable_testing()
    add_subdirectory(tests)
endif()
