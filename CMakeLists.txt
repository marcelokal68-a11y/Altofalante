cmake_minimum_required(VERSION 3.16)
project(Altofalante LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(dsp-core)
add_subdirectory(sync-core)
add_subdirectory(tools)

enable_testing()
add_test(NAME invariants COMMAND test_invariants)
add_test(NAME sync_sim COMMAND sync_sim)
