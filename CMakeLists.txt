cmake_minimum_required(VERSION 3.16)
project(mansion_seeds)

set(CMAKE_CXX_STANDARD 20)
set(ABSL_PROPAGATE_CXX_STD 1)
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
find_package(PkgConfig REQUIRED)
#pkg_check_modules(ABSL_HASHMAP REQUIRED IMPORTED_TARGET absl_flat_hash_map)
pkg_check_modules(FMT REQUIRED IMPORTED_TARGET fmt)

add_subdirectory(abseil-cpp)

add_executable(mansion_seeds main.cpp)
target_link_libraries(mansion_seeds PUBLIC PkgConfig::FMT absl::flat_hash_map)
