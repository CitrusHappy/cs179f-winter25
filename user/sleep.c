#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // use ticks at time measurement
    // get start time at function call,
        // wait(ticks)
    // check to see if time equals start time + inputted wait time (end time)
        // if equal or higher, continue execution

    if(argc <= 1) {
        printf("sleep: no arguments passed!\n");
        exit();
    }
    
    sleep(atoi(argv[1]));
    exit();
}