cmake_minimum_required(VERSION 3.16)
project(MCPS VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -march=native)
    endif()
elseif(MSVC)
    add_compile_options(/W4)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2)
    endif()
endif()

# Find OpenMP (optional)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found - enabling parallel rendering")
else()
    message(STATUS "OpenMP not found - rendering will be single-threaded")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Main executable
add_executable(mcps
    src/main.cpp
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(mcps PRIVATE OpenMP::OpenMP_CXX)
endif()

# Enable testing
enable_testing()

# Find Google Test (optional)
find_package(GTest QUIET)
if(GTest_FOUND)
    message(STATUS "Google Test found - building tests")
    
    add_executable(test_ray tests/test_ray.cpp)
    target_link_libraries(test_ray PRIVATE GTest::GTest GTest::Main)
    add_test(NAME TestRay COMMAND test_ray)
    
    add_executable(test_bvh tests/test_bvh.cpp)
    target_link_libraries(test_bvh PRIVATE GTest::GTest GTest::Main)
    add_test(NAME TestBVH COMMAND test_bvh)
    
    add_executable(test_materials tests/test_materials.cpp)
    target_link_libraries(test_materials PRIVATE GTest::GTest GTest::Main)
    add_test(NAME TestMaterials COMMAND test_materials)
    
    add_executable(test_integrator tests/test_integrator.cpp)
    target_link_libraries(test_integrator PRIVATE GTest::GTest GTest::Main)
    add_test(NAME TestIntegrator COMMAND test_integrator)
else()
    message(STATUS "Google Test not found - tests will not be built")
endif()

# Installation
install(TARGETS mcps DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
