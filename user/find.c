#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Attempts to derive a filename given a path
char *get_filename_from_path(const char *path) {
    const char *last_slash = 0; // Pointer to the last slash
    const char *p;

    for (p = path; *p != 0; p++) { // Traverse the string
        if (*p == '/') {
            last_slash = p; // Update last_slash when a '/' is found
        }
    }

    if (last_slash) {
        return (char *)(last_slash + 1); // Return the character after the last '/'
    }
    return (char *)path; // If no slash, return the whole path
}

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

    switch(stats.type){
    case T_FILE: // current object is a FILE
        printf("DEBUG: Found a file\n");
        //printf("DEBUG: %s %d %d %l\n", path, stats.type, stats.ino, stats.size);
        //printf("DEBUG: %s\n", get_filename_from_path(path));

        if(strcmp(get_filename_from_path(path), target) == 0) { // check if file name matches target
            printf("DEBUG: Found matching file!\n");
            printf("%s\n", path);
        }
    break;

    case T_DIR: // current object is a DIRECTORY
        char buffer[512]; // buffer 
        char *p; // allows us to manipulate the buffer
        struct dirent directory_entry; // a directory entry from a table containing [inum (inode number), file name]

        printf("DEBUG: Found a directory!\n");
        // add onto the directory
        strcpy(buffer, path); // copy path to buffer
        //printf("DEBUG: buf = %s\n", buffer);
        p = buffer + strlen(buffer); // sets pointer p to the end of the string in buffer, p is buffer now
        *p++ = '/'; // add slash at end of p and increment pointer
        //printf("DEBUG: buf = %s\n", buffer);

        // iterate through all files in directory
        // - each read() steps through the directory entries, returning the next one
        while(read(file_descriptor, &directory_entry, sizeof(directory_entry)) == sizeof(directory_entry)) {
            if(directory_entry.inum == 0 
                || strcmp(directory_entry.name, ".") == 0 
                || strcmp(directory_entry.name, "..") == 0
                ) // skip empty files, ".", and ".."
                continue;
            
            //printf("DEBUG: directory_entry.name = %s\n", directory_entry.name);
            
            memmove(p, directory_entry.name, DIRSIZ); // adds the current directory_entry.name to the end of the buffer
            p[DIRSIZ] = 0; // null terminate the buffer so it prints properly
            //printf("DEBUG: buf = %s\n", buffer);

            if(strcmp(directory_entry.name, target) == 0) { // check if file name matches target
                printf("DEBUG: Found matching file!\n");
                printf("%s\n", buffer);
            }

            find(buffer, target); // enter next directory (recursion)
        }
    break;
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