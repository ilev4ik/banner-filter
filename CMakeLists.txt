cmake_minimum_required(VERSION 2.8)
project(rambler_banners)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Add required flags (GCC & LLVM/Clang)
    target_compile_options(coverage_config INTERFACE
            -O0        # no optimization
            -g         # generate debug info
            --coverage # sets all required flags
            )
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
        target_link_options(coverage_config INTERFACE --coverage)
    else()
        target_link_libraries(coverage_config INTERFACE --coverage)
    endif()
endif(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

enable_testing()
add_subdirectory(test)

set(TARGET rambler_banners_${CMAKE_BUILD_TYPE})
add_executable(TARGET src/main.cpp ${HEADERS})