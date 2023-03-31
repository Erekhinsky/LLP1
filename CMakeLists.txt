cmake_minimum_required(VERSION 3.24)
project(llp1 C)

set(CMAKE_C_STANDARD 99)

add_executable(llp1 main.c src/node_internals.c src/scheme_operations.c src/database_operations.c src/node_operations.c src/query.c test.c src/condition.c)
