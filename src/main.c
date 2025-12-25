#include <FileManagement.h>
#include <stdio.h>
#include <time.h>

#include "../include/FileSystemStructure.h"
#include "../include/Directories.h"

/* TO DO:
   - add argument validation
   - optimise dir lookup
     - condence dir contents?
*/

int main(void) {
    format_disk("FS.bin", 10);

    fclose(fs.disk);
}
