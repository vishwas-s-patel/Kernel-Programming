/* -----------------------------------------------------------
 * File name   : sem.c
 * Description : 
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
#include "sem.h"


int main(int argc, char *argv[])
{
    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int i;
    pid_t pid, pids[N]; // process ids
    char semap_name_str[20];
    char status_str[20];
    semaphore_t *semap[N];
    semaphore_t *phil_mutex;
    semaphore_t *barrier;
    integer_t *integer;
    integer_t *status[N];
    char N_array[5];
    char M_array[5];
    char i_array[5];

    ///////////////////////////////////////////
    char count_name_str[10] = "counter";
    integer = integer_create(count_name_str, 0);

    integer_close(integer);

    phil_mutex = semaphore_create("mutex", 1);
    barrier = semaphore_create("barrier", 0);
    //////////////////////////////////////////


    sprintf(N_array, "%d", N);
    sprintf(M_array, "%d", M);

    for(i=0;i<N;i++)
    {
        pid = fork();
        if(pid==0)
        {
            // child
            printf("Inside child\n");

            sprintf(semap_name_str, "semaphore_%d", i);
            sprintf(status_str, "status_%d", i);

            //////////////////////////////////////
            // Create
            semap[i] = semaphore_create(semap_name_str, 0);
            status[i] = integer_create(status_str, 0);
            /////////////////////////////////////


            /////////////////////////////////////
            // Close
            integer_close(status[i]);
            semaphore_close(semap[i]);
            semaphore_close(phil_mutex);
            semaphore_close(barrier);
            /////////////////////////////////////
            //dining_philosophers(N, M, i, semap, phil_status);
            sprintf(i_array, "%d", i);

            execl("./phil", "./phil", N_array, M_array, i_array, NULL);

            printf("philosopher function error\n");
            exit(0);
        }
        else if(pid>0)
        {
            // parent
            pids[i] = pid;
            printf("pids[%d]=%d\n",i,pids[i]);
        }
        else
        {
            perror("fork");
            exit(0);
        }
    }

    // wait for child processes to end
    for(i=0;i<N;++i)
    {
        waitpid(pids[i],NULL,0);
    }

    //return 0;
    exit(0);
}
