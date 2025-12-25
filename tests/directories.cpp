// directories_test.cpp
// GoogleTest unit tests for directory helpers in Directories.c
//
// Build idea (example):
//   g++ -std=c++17 -I<gtest_include> directories_test.cpp -lgtest -lgtest_main -pthread
//
// IMPORTANT:
// - This file provides *stub implementations* of the dependencies your directory code calls
//   (read_inode/write_inode/alloc_direct_inode_block/alloc_block, plus globals fs/sb).
// - It uses a real FILE* backed by a temporary file as the “disk”.
// - Adjust include paths / constants if your project differs.

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

extern "C" {

// --------- Minimal constants / structs matching your code ---------
#define BLOCK_SIZE 4096
#define DIRECT_PTRS 12

#define FREE 0
#define USED 1

// you use INCREMENT/DECREMENT as "operation" in dir_block_update_count
#define INCREMENT (+1)
#define DECREMENT (-1)

#define IDIR  0x4000
#define IREG  0x8000

typedef struct {
    uint16_t mode;
    uint16_t links_count;
    uint32_t size;
    uint32_t direct[DIRECT_PTRS];
} Inode;

typedef struct {
    uint32_t inode_num;
    uint8_t  type;
    uint8_t  used;
    uint16_t _pad;
    char     name[28];
} DirEntry;

typedef struct {
    FILE *disk;
} FS;

typedef struct {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t data_block_start;
} Superblock;

// --------- Globals your code expects ---------
FS fs;
Superblock sb;

// --------- Forward decls of functions under test (your code) ---------
long dir_lookup(uint32_t dir_num, const char *entry_name);
long dir_add(uint32_t dir_inum, const char *name, uint32_t child_inum, uint8_t type);

long alloc_dir_entry(uint32_t dir_inum);
int  read_dir_entry(uint32_t offset, DirEntry *entry);
int  write_dir_entry(uint32_t offset, DirEntry *entry);

int      dir_block_full(uint32_t bnum);
int      dir_block_empty(uint32_t bnum);
uint32_t read_num_of_dir_entries(uint32_t bnum);
int      write_num_of_dir_entries(uint32_t bnum, uint32_t count);
int      dir_block_update_count(uint32_t bnum, int operation);

// --------- Stubs/mocks for external deps your code calls ---------
// These are intentionally simple and deterministic for unit tests.

static std::vector<Inode> g_inodes;
static uint32_t g_next_block = 1; // block 0 reserved / ignored

void read_inode(uint32_t inum, Inode *out) {
    *out = g_inodes.at(inum);
}

void write_inode(uint32_t inum, Inode *in) {
    g_inodes.at(inum) = *in;
}

// Allocate a fresh disk block number (absolute block number).
static uint32_t alloc_block() {
    return g_next_block++;
}

// Allocates a new block and attaches it to the first empty direct pointer.
// Also initializes the block to zeros and header count=0.
// Returns the allocated *block number* on success, -1 on failure.
int alloc_direct_inode_block(uint32_t dir_inum) {
    Inode dir;
    read_inode(dir_inum, &dir);

    int slot = -1;
    for (int i = 0; i < DIRECT_PTRS; i++) {
        if (dir.direct[i] == 0) { slot = i; break; }
    }
    if (slot == -1) return -1;

    uint32_t b = alloc_block();
    dir.direct[slot] = b;
    write_inode(dir_inum, &dir);

    // zero the whole block (so entry.used defaults to FREE)
    std::vector<uint8_t> zero(BLOCK_SIZE, 0);
    std::fseek(fs.disk, static_cast<long>(b) * BLOCK_SIZE, SEEK_SET);
    std::fwrite(zero.data(), 1, zero.size(), fs.disk);
    std::fflush(fs.disk);

    // ensure header count = 0
    write_num_of_dir_entries(b, 0);

    return static_cast<int>(b);
}

} // extern "C"

// --------- Bring in the code under test ---------
// For real projects, you’d #include "Directories.c" (or compile/link it separately).
// Here we paste your functions (verbatim except includes removed) so the test is self-contained.

extern "C" {

long dir_lookup(uint32_t dir_num, const char *entry_name) {
    Inode dir;
    read_inode(dir_num, &dir);

    for (uint32_t i = 0; i < DIRECT_PTRS; i++) {
        uint32_t block_num = dir.direct[i];

        if (block_num != 0) {
            for (uint32_t j = 0; j < BLOCK_SIZE/sizeof(DirEntry); j++) {
                DirEntry entry;
                uint32_t offset = block_num * BLOCK_SIZE + sizeof(uint32_t) + j * sizeof(DirEntry);
                read_dir_entry(offset, &entry);

                if (entry.used == FREE) return -1;
                if (strcmp(entry.name, entry_name) == 0) {
                    return entry.inode_num;
                }
            }
        }
    }
    return -1;
}

long dir_add(uint32_t dir_inum, const char *name, uint32_t child_inum, uint8_t type) {
    if (dir_lookup(dir_inum, name) != -1) return -1;

    DirEntry entry = {0};
    entry.inode_num = child_inum;
    entry.type = type;
    entry.used = USED;
    strncpy(entry.name, name, sizeof(entry.name)-1);

    long dir_entry_address = alloc_dir_entry(dir_inum);
    if (dir_entry_address == -1) return -1;

    write_dir_entry(static_cast<uint32_t>(dir_entry_address), &entry);

    Inode dir;
    read_inode(dir_inum, &dir);
    dir.size += sizeof(DirEntry);
    write_inode(dir_inum, &dir);

    return dir_entry_address;
}

long alloc_dir_entry(uint32_t dir_inum) {
    Inode dir;
    read_inode(dir_inum, &dir);

    for (int i = 0; i < DIRECT_PTRS; i++) {
        if (dir.direct[i] == 0) continue;
        if (dir_block_full(dir.direct[i])) continue;

        long new_entry_address = (long)dir.direct[i] * BLOCK_SIZE
                + (long)sizeof(uint32_t)
                + (long)read_num_of_dir_entries(dir.direct[i]) * (long)sizeof(DirEntry);

        dir_block_update_count(dir.direct[i], INCREMENT);

        return new_entry_address;
    }

    int bnum = alloc_direct_inode_block(dir_inum);
    if (bnum > -1 ) {
        long new_entry_address = (long)bnum * BLOCK_SIZE + (long)sizeof(uint32_t);
        dir_block_update_count((uint32_t)bnum, INCREMENT);
        return new_entry_address;
    }

    return -1;
}

int read_dir_entry(uint32_t offset, DirEntry *entry) {
    std::fseek(fs.disk, (long)offset, SEEK_SET);
    std::fread(entry, sizeof(DirEntry), 1, fs.disk);
    return 0;
}

int write_dir_entry(uint32_t offset, DirEntry *entry) {
    std::fseek(fs.disk, (long)offset, SEEK_SET);
    std::fwrite(entry, sizeof(DirEntry), 1, fs.disk);
    std::fflush(fs.disk);
    return 0;
}

int dir_block_full(uint32_t bnum) {
    uint32_t num_of_entries = read_num_of_dir_entries(bnum);
    if (num_of_entries == (BLOCK_SIZE - sizeof(uint32_t)) / sizeof(DirEntry)) return 1;
    return 0;
}

int dir_block_empty(uint32_t bnum) {
    uint32_t num_of_entries = read_num_of_dir_entries(bnum);
    if (num_of_entries == 0) return 1;
    return 0;
}

uint32_t read_num_of_dir_entries(uint32_t bnum) {
    uint32_t num_of_entries = 0;
    std::fseek(fs.disk, (long)bnum * BLOCK_SIZE, SEEK_SET);
    std::fread(&num_of_entries, sizeof(uint32_t), 1, fs.disk);
    return num_of_entries;
}

int write_num_of_dir_entries(uint32_t bnum, uint32_t count) {
    std::fseek(fs.disk, (long)bnum * BLOCK_SIZE, SEEK_SET);
    std::fwrite(&count, sizeof(uint32_t), 1, fs.disk);
    std::fflush(fs.disk);
    return 0;
}

int dir_block_update_count(uint32_t bnum, int operation) {
    uint32_t count = read_num_of_dir_entries(bnum);
    count += operation;
    write_num_of_dir_entries(bnum, count);
    return 0;
}

} // extern "C"

// --------------------- Test fixture ---------------------

class DirectoriesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temp disk file
        fs.disk = std::tmpfile();
        ASSERT_NE(fs.disk, nullptr);

        // Prepare superblock-ish values (only used by stubs if you extend)
        sb.total_inodes = 64;
        sb.total_blocks = 256;
        sb.data_block_start = 1;

        // Initialize inode table in memory
        g_inodes.assign(sb.total_inodes, Inode{});
        g_next_block = sb.data_block_start;

        // Pre-extend disk file to some blocks of zeros (so fseek+fread works predictably)
        std::vector<uint8_t> zero(BLOCK_SIZE, 0);
        for (int i = 0; i < 64; i++) {
            std::fwrite(zero.data(), 1, zero.size(), fs.disk);
        }
        std::fflush(fs.disk);

        // Create root directory inode 0
        g_inodes[0].mode = IDIR;
        g_inodes[0].links_count = 2;
        g_inodes[0].size = 0;
        std::memset(g_inodes[0].direct, 0, sizeof(g_inodes[0].direct));

        // Give root one directory block so lookup/add have somewhere to work,
        // and init it properly (zero + count=0).
        uint32_t root_b = alloc_block();
        g_inodes[0].direct[0] = root_b;

        std::vector<uint8_t> blk(BLOCK_SIZE, 0);
        std::fseek(fs.disk, (long)root_b * BLOCK_SIZE, SEEK_SET);
        std::fwrite(blk.data(), 1, blk.size(), fs.disk);
        std::fflush(fs.disk);
        write_num_of_dir_entries(root_b, 0);
    }

    void TearDown() override {
        if (fs.disk) std::fclose(fs.disk);
        fs.disk = nullptr;
        g_inodes.clear();
    }

    uint32_t root_block() const { return g_inodes[0].direct[0]; }

    // Helper: write an entry at logical index i (without changing header),
    // useful for isolated lookup tests.
    void write_entry_at(uint32_t bnum, uint32_t idx, const char* name, uint32_t inum, uint8_t type, uint8_t used) {
        DirEntry e{};
        e.inode_num = inum;
        e.type = type;
        e.used = used;
        std::strncpy(e.name, name, sizeof(e.name) - 1);

        uint32_t off = bnum * BLOCK_SIZE + (uint32_t)sizeof(uint32_t) + idx * (uint32_t)sizeof(DirEntry);
        write_dir_entry(off, &e);
    }
};

// --------------------- Tests ---------------------

TEST_F(DirectoriesTest, ReadWriteNumDirEntries_Works) {
    uint32_t b = root_block();
    EXPECT_EQ(read_num_of_dir_entries(b), 0u);

    EXPECT_EQ(write_num_of_dir_entries(b, 3), 0);
    EXPECT_EQ(read_num_of_dir_entries(b), 3u);

    EXPECT_EQ(write_num_of_dir_entries(b, 0), 0);
    EXPECT_EQ(read_num_of_dir_entries(b), 0u);
}

TEST_F(DirectoriesTest, DirBlockUpdateCount_IncrementsAndDecrements) {
    uint32_t b = root_block();

    EXPECT_EQ(read_num_of_dir_entries(b), 0u);
    dir_block_update_count(b, INCREMENT);
    EXPECT_EQ(read_num_of_dir_entries(b), 1u);

    dir_block_update_count(b, INCREMENT);
    EXPECT_EQ(read_num_of_dir_entries(b), 2u);

    dir_block_update_count(b, DECREMENT);
    EXPECT_EQ(read_num_of_dir_entries(b), 1u);
}

TEST_F(DirectoriesTest, DirBlockEmptyAndFull_Basics) {
    uint32_t b = root_block();

    EXPECT_EQ(dir_block_empty(b), 1);
    EXPECT_EQ(dir_block_full(b), 0);

    // Put it at max
    uint32_t max_entries = (BLOCK_SIZE - (uint32_t)sizeof(uint32_t)) / (uint32_t)sizeof(DirEntry);
    write_num_of_dir_entries(b, max_entries);

    EXPECT_EQ(dir_block_empty(b), 0);
    EXPECT_EQ(dir_block_full(b), 1);
}

TEST_F(DirectoriesTest, AllocDirEntry_AppendsInExistingBlockAndUpdatesCount) {
    uint32_t b = root_block();

    EXPECT_EQ(read_num_of_dir_entries(b), 0u);

    long off0 = alloc_dir_entry(0);
    ASSERT_GE(off0, 0);
    EXPECT_EQ((uint32_t)(off0 / BLOCK_SIZE), b);
    EXPECT_EQ(read_num_of_dir_entries(b), 1u);

    long off1 = alloc_dir_entry(0);
    ASSERT_GE(off1, 0);
    EXPECT_EQ((uint32_t)(off1 / BLOCK_SIZE), b);
    EXPECT_EQ(read_num_of_dir_entries(b), 2u);

    // second offset should be exactly one DirEntry after the first
    EXPECT_EQ(off1 - off0, (long)sizeof(DirEntry));
}

TEST_F(DirectoriesTest, AllocDirEntry_AllocatesNewBlockWhenCurrentFull) {
    uint32_t b = root_block();
    uint32_t max_entries = (BLOCK_SIZE - (uint32_t)sizeof(uint32_t)) / (uint32_t)sizeof(DirEntry);

    // Fill current directory block (header only) so alloc_dir_entry must allocate a new one
    write_num_of_dir_entries(b, max_entries);
    EXPECT_EQ(dir_block_full(b), 1);

    long off = alloc_dir_entry(0);
    ASSERT_GE(off, 0);

    uint32_t used_block = (uint32_t)(off / BLOCK_SIZE);
    EXPECT_NE(used_block, b);                 // new block
    EXPECT_EQ(read_num_of_dir_entries(used_block), 1u); // count incremented in new block
    EXPECT_NE(g_inodes[0].direct[1], 0u);      // attached to inode
}

TEST_F(DirectoriesTest, DirAddThenLookup_FindsEntry) {
    // Ensure root block is empty + zeroed
    uint32_t b = root_block();
    EXPECT_EQ(read_num_of_dir_entries(b), 0u);

    long addr = dir_add(0, "bin", 5, IDIR);
    ASSERT_GE(addr, 0);

    // lookup should find it
    EXPECT_EQ(dir_lookup(0, "bin"), 5);

    // size should have increased by one entry
    EXPECT_EQ(g_inodes[0].size, (uint32_t)sizeof(DirEntry));
}

TEST_F(DirectoriesTest, DirAdd_DuplicateNameFails) {
    long addr1 = dir_add(0, "etc", 7, IDIR);
    ASSERT_GE(addr1, 0);

    long addr2 = dir_add(0, "etc", 8, IDIR);
    EXPECT_EQ(addr2, -1);
}

TEST_F(DirectoriesTest, DirLookup_StopsAtFirstFreeEntry) {
    uint32_t b = root_block();

    // Write one USED entry, then a FREE entry; lookup for later name should fail due to early stop.
    write_entry_at(b, 0, "first", 11, IREG, USED);
    write_entry_at(b, 1, "second", 22, IREG, FREE);

    // dir_lookup scans entries and returns -1 on first FREE.
    EXPECT_EQ(dir_lookup(0, "second"), -1);
    EXPECT_EQ(dir_lookup(0, "first"), 11);
}

TEST_F(DirectoriesTest, ReadWriteDirEntry_RoundTrip) {
    uint32_t b = root_block();

    DirEntry e{};
    e.inode_num = 123;
    e.type = IREG;
    e.used = USED;
    std::strncpy(e.name, "hello.txt", sizeof(e.name)-1);

    uint32_t off = b * BLOCK_SIZE + (uint32_t)sizeof(uint32_t);
    ASSERT_EQ(write_dir_entry(off, &e), 0);

    DirEntry out{};
    ASSERT_EQ(read_dir_entry(off, &out), 0);

    EXPECT_EQ(out.inode_num, 123u);
    EXPECT_EQ(out.type, (uint8_t)IREG);
    EXPECT_EQ(out.used, (uint8_t)USED);
    EXPECT_STREQ(out.name, "hello.txt");
}

