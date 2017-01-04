/* -----------------------------------------------------------
 * File name   : combiner.c
 * Description : Creates the child processes mapper and reducer,
 *               and make it communicate with each other using
 *               PIPEs. The mapper writes to stdout and reducer
 *               reads from the stdout. 
 * Author      : Vishwas Satish Patel
 * -----------------------------------------------------------
*/

/*
 * -----------------------------------------------------------
 * Include section
 * -----------------------------------------------------------
 */
 
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * -----------------------------------------------------------
 * MACRO (define) section
 * -----------------------------------------------------------
 */
 
#define     READ_END            0
#define     WRITE_END           1

/*
 * -----------------------------------------------------------
 * Type definition section
 * -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------
 * Global prototypes section
 * -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------
 * Local prototypes section
 * -----------------------------------------------------------
 */
 
/*
 * -----------------------------------------------------------
 * Global data section
 * -----------------------------------------------------------
 */

 /*
 * -----------------------------------------------------------
 * Local (static) data section
 * -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------
 * Local (static) and inline functions section
 * -----------------------------------------------------------
 */

int main(int argc, char *argv[])
{
    pid_t pid1, pid2;
    int file_des[2];
    int status;
    
    pipe(file_des);

    pid1 = fork();

    switch (pid1)
    {
        case -1:
            // Error creating fork
            printf("Fork error\n");
        case 0:
        {
            // Child Process
            close(file_des[WRITE_END]);
            dup2(file_des[READ_END], STDIN_FILENO);
            execlp("./reducer", "reducer",  NULL);

            printf("reducer error\n");
        }
        
        default: //parent goes through
            break;

    }

    pid2 = fork();

    switch (pid2)
    {
        case -1:
            // Error creating fork
            printf("Fork error\n");
        case 0:
        {
            // Child Process
            close(file_des[READ_END]);
            dup2(file_des[WRITE_END], STDOUT_FILENO);
            execlp("./mapper", "mapper", argv[1], NULL);

            printf("mapper error\n");
        }
        
        default: //parent goes through
            break;

    }
    
    // Parent Process
    close(file_des[READ_END]);
    close(file_des[WRITE_END]);
    
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);

    exit(0);
}

/*
 * -----------------------------------------------------------
 * End of File
 * -----------------------------------------------------------
 */
