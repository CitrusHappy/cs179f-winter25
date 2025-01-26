#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/proc.h"
#include "kernel/param.h" // For MAXARG


#define MAX_LINE 1024 // Maximum length of a line

void run_command(char *command, char *arg) {

}

int main(int argc, char *argv[]) {
    if (argc < 2) { // must have atleast one argument
        printf("Usage: xargs <command>\n");
        exit();
    }

    // Concatenate the command from the arguments
    char* command[MAX_LINE];
    for (int i = 1; i < argc; i++) {
        strcat(command, argv[i]);
        if (i < argc - 1) {
            strcat(command, " ");
        }
    }



    return 0;
}
