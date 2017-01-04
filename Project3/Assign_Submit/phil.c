/* -----------------------------------------------------------
 * File name   : reducer.c
 * Description : Gets the word key_value pairs from mapper in the
 *               format (word,1). It counts the words and generates
 *               (word, count).
 * Author      : Vishwas Satish Patel
 * -----------------------------------------------------------
*/

/*
 * -----------------------------------------------------------
 * Include section
 * -----------------------------------------------------------
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sem.h"

/*
 * -----------------------------------------------------------
 * MACRO (define) section
 * -----------------------------------------------------------
 */
 
#define     I_TH_PHIL       0
#define     LEFT_PHIL       1
#define     RIGHT_PHIL      2

#define     HUNGRY          0
#define     THINKING        1
#define     EATING          2

/*
 * -----------------------------------------------------------
 * Global data section
 * -----------------------------------------------------------
 */
int no_of_philosophers;
int phil_no;
int left, right;

/*
 * -----------------------------------------------------------
 * Local prototypes section
 * -----------------------------------------------------------
 */
void test(int phil_number);
void take_forks(int phil_number);
void drop_forks(int phil_number);

/*
 * -----------------------------------------------------------
 * Function     : test
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
void test(int phil_number)
{
    int         left;
    int         right;

    char        semap_name_str[20];
    char        status_str[20];
    semaphore_t *semap[3];
    semaphore_t *phil_mutex;
    integer_t   *status[3];

    left = (phil_number - 1 + no_of_philosophers) % no_of_philosophers;
    right = (phil_number + 1) % no_of_philosophers;

    sprintf(semap_name_str, "semaphore_%d", phil_number);
    sprintf(status_str, "status_%d", phil_number);

    phil_mutex = semaphore_open("mutex");

    semap[I_TH_PHIL] = semaphore_open(semap_name_str);
    status[I_TH_PHIL] = integer_open(status_str);

    sprintf(semap_name_str, "semaphore_%d", left);
    sprintf(status_str, "status_%d", left);

    semap[LEFT_PHIL] = semaphore_open(semap_name_str);
    status[LEFT_PHIL] = integer_open(status_str);

    sprintf(semap_name_str, "semaphore_%d", right);
    sprintf(status_str, "status_%d", right);

    semap[RIGHT_PHIL] = semaphore_open(semap_name_str);
    status[RIGHT_PHIL] = integer_open(status_str);


    if(status[I_TH_PHIL]->count == HUNGRY && status[LEFT_PHIL]->count != EATING && status[RIGHT_PHIL]->count != EATING)
    {
        printf("Philosopher %d has taken both forks and started EATING\n", phil_number);
        status[I_TH_PHIL]->count = EATING;
        semaphore_post(semap[I_TH_PHIL]);
    }
}

/*
 * -----------------------------------------------------------
 * Function     : take_forks
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
void take_forks(int phil_number)
{
    char        semap_name_str[20];
    char        status_str[20];
    semaphore_t *semap;
    semaphore_t *phil_mutex;
    integer_t   *status;

    sprintf(semap_name_str, "semaphore_%d", phil_number);
    sprintf(status_str, "status_%d", phil_number);

    phil_mutex = semaphore_open("mutex");
    semap = semaphore_open(semap_name_str);
    status = integer_open(status_str);

    semaphore_wait(phil_mutex);
    printf("Philosopher %d is HUNGRY and testing to take forks\n", phil_number);
    status->count = HUNGRY;
    test(phil_number);
    semaphore_post(phil_mutex);

    semaphore_wait(semap);
}

/*
 * -----------------------------------------------------------
 * Function     : drop_forks
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
void drop_forks(int phil_number)
{
    int         left;
    int         right;
    char        status_str[20];

    semaphore_t *phil_mutex;
    integer_t   *status;

    phil_mutex = semaphore_open("mutex");

    sprintf(status_str, "status_%d", phil_number);
    status = integer_open(status_str);

    left = (phil_number - 1 + no_of_philosophers) % no_of_philosophers;
    right = (phil_number + 1) % no_of_philosophers;

    ///////////////////////////////////////
    semaphore_wait(phil_mutex);
    status->count = THINKING;
    printf("Philosopher %d is dropping the forks and giving the chance for other philosophers to eat\n", phil_number);

    test(left);
    test(right);
    semaphore_post(phil_mutex);
    ///////////////////////////////////////
}



int main(int argc, char *argv[])
{
    int         N = atoi(argv[1]);
    int         M = atoi(argv[2]);
    int         i;
    int         r;
    char        count_name_str[10] = "counter";

    semaphore_t *phil_mutex;
    semaphore_t *barrier;
    integer_t   *integer;

    phil_no = atoi(argv[3]);
    no_of_philosophers = N;

    srand (time(NULL) + phil_no);
    //////////////////////////////////////////

    integer = integer_open(count_name_str);
    phil_mutex = semaphore_open("mutex");
    barrier = semaphore_open("barrier");

    ///////////////////////////////////////////
    semaphore_wait(phil_mutex);
        integer->count++;
    semaphore_post(phil_mutex);
    if (integer->count == N)
        semaphore_post(barrier);

    printf("Philosopher %d is waiting in barrier\n", phil_no);
    semaphore_wait(barrier);
    semaphore_post(barrier);

    r = (unsigned)rand() % 65536;
    usleep(r * 10);                 /// THINK for random amount of time
    
    printf("All the philsophers have reached barrier, they can start EATING\n");
    /////////////////////////////////////////

    for(i = 0; i < M; i++)
    {
        r = (unsigned)rand() % 65536;
        printf("Philosopher %d THINKING for %d useconds in iteration no %d\n", phil_no, r *10, i);
        usleep(r * 10);                 /// THINK for random amount of time

        take_forks(phil_no);

        r = rand() % 65536;
        printf("Philosopher %d EATING for %d useconds in iteration no %d\n", phil_no, r *10, i);
        usleep(r * 10);                 /// EAT for random amount of time

        drop_forks(phil_no);
    }

    //////////////////////////////////////////
    integer_close(integer);
    semaphore_close(phil_mutex);
    semaphore_close(barrier);

}
