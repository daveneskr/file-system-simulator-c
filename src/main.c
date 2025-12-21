#include <stdio.h>

#include "../include/FileSystemStructure.h"

/* TO DO:
   - add argument validation
*/

int main(void) {
    format_disk("FS.bin", 10);

    fclose(fs.disk);
}
