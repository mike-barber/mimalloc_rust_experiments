cmake_minimum_required(VERSION 3.14)
project(cpp-bench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Add mimalloc as a subdirectory
add_subdirectory(../libmimalloc-sys/c_src/mimalloc/v3 ${CMAKE_CURRENT_BINARY_DIR}/mimalloc)
#add_subdirectory(../libmimalloc-sys/c_src/mimalloc/v2 ${CMAKE_CURRENT_BINARY_DIR}/mimalloc)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Create executable
add_executable(cpp-bench src/cpp-bench.cpp)

# links
#target_link_libraries(cpp-bench PRIVATE mimalloc)
target_link_libraries(cpp-bench mimalloc Threads::Threads)


