cmake_minimum_required(VERSION 3.16)
project(FileSystemSimulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(fs_core
        src/FileSystemStructure.c
        src/FileManagement.c
        src/Directories.c
)

target_include_directories(fs_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(fs_cli src/main.c)
target_link_libraries(fs_cli PRIVATE fs_core)
