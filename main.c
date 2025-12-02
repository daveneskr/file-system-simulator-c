#include "FileManagement.h"
#include "FileSystemStructure.h"

int main(void) {
    format_disk("FS.bin", 10);

    fclose(disk);
}