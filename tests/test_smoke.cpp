//
// Created by David Neškrabal on 22.12.2025.
//
#include <gtest/gtest.h>

extern "C" {
#include "FileSystemStructure.h"
#include "FileManagement.h"
#include "Directories.h"
}

TEST(Smoke, HeadersLink) {
    // If this links, your C code + headers are wired correctly.
    SUCCEED();
}

