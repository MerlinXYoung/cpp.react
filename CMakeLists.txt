### Configuration
cmake_minimum_required (VERSION 2.6)

project (CppReact)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wpedantic -Wno-#pragma-messages -Wno-unused-variable")

include_directories ("${PROJECT_SOURCE_DIR}/include")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/3rd/oneTBB/include")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/3rd/oneTBB/build/linux_intel64_gcc_cc7.4.0_libc2.27_kernel4.4.0_release")

### CppReact
add_library(CppReact 
	src/detail/graph_impl.cpp)

target_link_libraries(CppReact tbb)

### examples/ 
option(build_examples "Build examples?" ON)
if(build_examples)
	add_subdirectory(examples)
endif()

### benchmarks/ 
option(build_benchmarks "Build benchmarks?" OFF)
if(build_benchmarks)
	add_subdirectory(benchmarks)
endif()

### tests/ 
option(build_tests "Build unit tests?" OFF)
if(build_tests)
	add_subdirectory(tests)
endif()
