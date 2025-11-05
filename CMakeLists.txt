cmake_minimum_required(VERSION 4.0)
project(semaphore_client)

set(CMAKE_C_STANDARD 11)

add_executable(
        semaphore_client
        main.c
        client_semaphore.c
)

target_link_libraries(
        semaphore_client
        ${CMAKE_SOURCE_DIR}/libseg.a
)