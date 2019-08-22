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
        ${INCLUDE_DIR}/tuple_utils.h
        ${INCLUDE_DIR}/banner_traits.h
        ${INCLUDE_DIR}/banner.h
        ${INCLUDE_DIR}/banner_ex.h
        ${INCLUDE_DIR}/filter.h
        ${INCLUDE_DIR}/lyciator.h
        )

enable_testing()
add_subdirectory(test)

set(TARGETNAME rambler_banners_${CMAKE_BUILD_TYPE})
add_executable(${TARGETNAME} src/main.cpp ${HEADERS})
target_compile_options(${TARGETNAME} PRIVATE --coverage)