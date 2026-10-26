cmake_minimum_required(VERSION 3.16)
project(BadBishop C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Performance flags
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0")
endif()

include_directories(src)

# Core engine source files
set(ENGINE_SOURCES
    src/init.c
    src/bitboards.c
    src/hashkeys.c
    src/board.c
    src/attack.c
    src/io.c
)

# Main Executable
add_executable(badbishop src/main.c ${ENGINE_SOURCES})

# Enable testing
enable_testing()
add_executable(badbishop_tests tests/test_perft.c ${ENGINE_SOURCES})
add_test(NAME PerftTest COMMAND badbishop_tests)