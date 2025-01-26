#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h" // For MAXARG

//TODO: for some reason the debug print statement doesnt correctly read out the initial_cmd after modifying it. 
// might be a sign of unexpected behavior?

void run_command(char **initial_cmd, char *piped_input, int initial_cmd_count) {
    /*
    printf("DEBUG: Running command with parameters:\n");
    for(int i = 0; initial_cmd[i] != 0; i++) {
        printf("DEBUG: initial_cmd[%d] - %s\n", i, initial_cmd[i]);
    }
    printf("DEBUG: arg - %s\n", piped_input);
    printf("DEBUG: initial_cmd_count - %d\n", initial_cmd_count);
    */

    // adds the additional argument from the input line
    initial_cmd[initial_cmd_count++] = piped_input;
    initial_cmd[initial_cmd_count] = 0; // Null-terminate the argv array

    /*
    for(int i = 0; initial_cmd[i] != 0; i++) {
            printf("DEBUG: initial_cmd[%d] - %s\n", i, initial_cmd[i]);
    }
    */

    // fork a child process to execute the command
    int pid = fork();
    if (pid < 0) {
        printf("ERROR: xargs: fork()\n");
        exit();
    } else if (pid == 0) {
        // In child process, execute the command
        //printf("DEBUG: CHILD PROCESS pid - %d\n", pid);
        
        exec(initial_cmd[0], initial_cmd); //(path of executable aka executable name, arguments)
        printf("ERROR: xargs: exec()\n");
        exit();
    } else {
        //printf("DEBUG: PARENT PROCESS pid - %d\n", pid);
        // In parent process, wait for the child to complete
        wait();
        //printf("DEBUG: child_pid - %d\n", child_pid);
    }
}

int main(int argc, char *argv[]) {
    /*
    for(int i = 0; i < argc; i++) {
            printf("DEBUG: argv[%d] - %s\n", i, argv[i]);
    }
    */

    if (argc < 2) { // must have atleast one argument
        printf("Usage: xargs <command>\n");
        exit();
    }

    // read() input lines one char at a time
    char line[MAX_LINE];
    int index = 0;
    while (1) {
        char c;
        int bytes_read = read(0, &c, 1); // 0 for stdin
        //printf("DEBUG: %c\n", c);
        //printf("DEBUG: index before - %d\n", index);
        if (bytes_read <= 0) { // End of piped input or error
            //printf("DEBUG: End of piped input\n");
            if (index > 0) {
                line[index] = 0; // null terminate the line
                run_command(argv + 1, line, argc - 1);
            }
            break;
        } else if (c == '\n') {
            //printf("DEBUG: End of line found\n");
            line[index] = 0; // null terminate the line
            run_command(argv + 1, line, argc - 1);
            index = 0; // reset for the next line
        } else if (index < MAX_LINE - 1) { // if index not at the end of the line
            line[index++] = c; // increment index
        }
        //printf("DEBUG: index after - %d\n", index);
    }

    return 0;
}
