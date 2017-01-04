/* -----------------------------------------------------------
 * File name   : sem.h
 * Description : 
 * Author      : Vishwas Satish Patel
 * -----------------------------------------------------------
*/

/*
 * -----------------------------------------------------------
 * Include section
 * -----------------------------------------------------------
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * -----------------------------------------------------------
 * MACRO (define) section
 * -----------------------------------------------------------
 */
/*
 * -----------------------------------------------------------
 * Type definition section
 * -----------------------------------------------------------
 */
struct semaphore
{
    pthread_mutex_t lock;
    pthread_cond_t  nonzero;
    unsigned        count;
};

struct integer
{
    int count;
};

typedef struct semaphore    semaphore_t;
typedef struct integer      integer_t;

/*
 * -----------------------------------------------------------
 * Local prototypes section
 * -----------------------------------------------------------
 */
semaphore_t *semaphore_create(char *semaphore_name, int init_count);
semaphore_t *semaphore_open(char *semaphore_name);
void semaphore_post(semaphore_t *semap);
void semaphore_wait(semaphore_t *semap);
void semaphore_close(semaphore_t *semap);

integer_t *integer_create(char *integer_name, int init_count);
integer_t *integer_open(char *integer_name);
integer_t *integer_close(integer_t *integer);


