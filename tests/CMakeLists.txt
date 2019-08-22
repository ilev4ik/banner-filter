find_package(GTest REQUIRED)

add_executable(banner_tests banner_tests.cpp)
target_include_directories(banner_tests PRIVATE
        ${INCLUDE_DIR}
        ${GTEST_INCLUDE_DIRS}
        )
target_link_libraries(banner_tests  GTest::GTest GTest::Main coverage_config)

add_test(UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/banner_tests)
