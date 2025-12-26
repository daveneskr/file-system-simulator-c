cmake_minimum_required(VERSION 3.16)
project(FileSystemSimulator C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core C code compiled once
add_library(fs_core
        src/FileSystemStructure.c
        src/FileManagement.c
        src/Directories.c
        src/Inode.c
)

target_include_directories(fs_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Your main program
add_executable(fs_cli src/main.c
        include/Inode.h)
target_link_libraries(fs_cli PRIVATE fs_core)

# Tests
add_subdirectory(tests)
