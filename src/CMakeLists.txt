set(TARGETNAME rambler_banners_${CMAKE_BUILD_TYPE})

add_executable(${TARGETNAME} main.cpp)
target_include_directories(${TARGETNAME} PUBLIC ${INCLUDE_DIR})
target_compile_options(${TARGETNAME} PUBLIC -Wall -pedantic)