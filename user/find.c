#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *target) {
    int file_descriptor; // an identifier # for a file/dir, so we can lookup it's directory_entry
    struct stat stats; // file size, permissions, timestamps, file type, and inode number.
    // inode number: unique id for the file, used by system to locate a file's metadata

    // stores path as an int into file_descriptor 
    if((file_descriptor = open(path, 0)) < 0) { // open 0 : read_only mode
        fprintf(2, "ERROR: find: cannot open %s\n", path);
        return;
    }

    // get stats from input (file_descriptor)
    if(fstat(file_descriptor, &stats) < 0) { // fstat inputs a file_descriptor to return stats, stat inputs a path
        fprintf(2, "ERROR: find: cannot stat %s\n", path);
        close(file_descriptor);
        return;
    }

    // Ensure it's a directory before proceeding
    if (stats.type != T_DIR) {
        fprintf(2, "ERROR: find: %s is not a directory\n", path);
        close(file_descriptor);
        return;
    }

    char buffer[512]; // buffer 
    char *p; // allows us to manipulate the buffer
    struct dirent directory_entry; // a directory entry from a table containing [inum (inode number), file name]

    // add onto the buffer
    strcpy(buffer, path); // copy path to buffer
    p = buffer + strlen(buffer); // sets pointer p to the end of the string in buffer, p is buffer now
    *p++ = '/'; // add slash at end of p and increment pointer
    //printf("DEBUG: Found a directory - %s\n", buffer);

    // iterate through all files in directory
    // - each read() steps through the directory entries, returning the next one
    while(read(file_descriptor, &directory_entry, sizeof(directory_entry)) == sizeof(directory_entry)) {
        if(directory_entry.inum == 0 
            || strcmp(directory_entry.name, ".") == 0 
            || strcmp(directory_entry.name, "..") == 0
            ) // skip empty files, ".", and ".."
            continue;
        
        memmove(p, directory_entry.name, DIRSIZ); // adds the current directory_entry.name to the end of the buffer
        p[DIRSIZ] = 0; // null terminate the buffer so it prints properly
        //printf("DEBUG: buf = %s\n", buffer);

        // Get the status of the current entry
        if (stat(buffer, &stats) < 0) {
            fprintf(2, "ERROR: find: cannot stat %s\n", buffer);
            continue;
        }

        //printf("DEBUG: %s matches %s?\n", directory_entry.name, target);
        if(strcmp(directory_entry.name, target) == 0) { // check if file name matches target
            //printf("DEBUG: MATCHED\n");
            printf("%s\n", buffer);
        }

        if(stats.type == T_DIR) {
            find(buffer, target); // enter next directory (recursion)
        }
    }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: find <path> <filename>\n");
    exit();
  }

  find(argv[1], argv[2]);
  exit();
}