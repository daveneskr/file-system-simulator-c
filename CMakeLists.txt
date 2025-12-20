cmake_minimum_required(VERSION 3.30)
project(untitled C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled src/main.c
        include/FileSystemStructure.h
        src/FileSystemStructure.c
        src/FileManagement.c
        include/FileManagement.h
        src/Directories.c
        include/Directories.h
)
