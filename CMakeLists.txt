cmake_minimum_required(VERSION 4.0)
project(semaphore_client)

set(CMAKE_CXX_STANDARD 20)

add_executable(semaphore_client main.cpp)

target_link_libraries(semaphore_client ${CMAKE_SOURCE_DIR}/libseg.a)