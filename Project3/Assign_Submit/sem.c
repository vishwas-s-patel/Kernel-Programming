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
 
#include <stdio.h>
#include <stdlib.h>
#include "sem.h"


/*
 * -----------------------------------------------------------
 * Function     : integer_create
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
 
integer_t *integer_create(char *integer_name, int init_count)
{
    int fd;
    integer_t *integer;

    fd = open(integer_name, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
        return (NULL);

    (void) ftruncate(fd, sizeof(integer_t));

    integer = (integer_t *) mmap(NULL, sizeof(integer_t),
            PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, 0);

    close(fd);
    integer->count = init_count;

    return(integer);
}


/*
 * -----------------------------------------------------------
 * Function     : integer_open
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
integer_t *integer_open(char *integer_name)
{
    int fd;
    integer_t *integer;

    fd = open(integer_name, O_RDWR, 0666);
    if (fd < 0)
        return (NULL);


    integer = (integer_t *) mmap(NULL, sizeof(integer_t),
            PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, 0);

    close (fd);
    return (integer);
}

/*
 * -----------------------------------------------------------
 * Function     : integer_close
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
integer_t *integer_close(integer_t *integer)
{
    munmap((void *)integer, sizeof(integer_t));
}

/*
 * -----------------------------------------------------------
 * Function     : semaphore_create
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
semaphore_t *semaphore_create(char *semaphore_name, int init_count)
{
    int fd;
    semaphore_t *semap;
    pthread_mutexattr_t psharedm;
    pthread_condattr_t psharedc;

    fd = open(semaphore_name, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
        return (NULL);
    (void) ftruncate(fd, sizeof(semaphore_t));
    (void) pthread_mutexattr_init(&psharedm);
    (void) pthread_mutexattr_setpshared(&psharedm,
        PTHREAD_PROCESS_SHARED);
    (void) pthread_condattr_init(&psharedc);
    (void) pthread_condattr_setpshared(&psharedc,
        PTHREAD_PROCESS_SHARED);
    semap = (semaphore_t *) mmap(NULL, sizeof(semaphore_t),
            PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, 0);
    close (fd);
    (void) pthread_mutex_init(&semap->lock, &psharedm);
    (void) pthread_cond_init(&semap->nonzero, &psharedc);
    semap->count = init_count;
    return (semap);
}

/*
 * -----------------------------------------------------------
 * Function     : semaphore_open
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */
semaphore_t *semaphore_open(char *semaphore_name)
{
    int fd;
    semaphore_t *semap;


    fd = open(semaphore_name, O_RDWR, 0666);
    if (fd < 0)
        return (NULL);
    semap = (semaphore_t *) mmap(NULL, sizeof(semaphore_t),
            PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, 0);
    close (fd);
    return (semap);
}

/*
 * -----------------------------------------------------------
 * Function     : semaphore_post
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */

void semaphore_post(semaphore_t *semap)
{
    pthread_mutex_lock(&semap->lock);
    if (semap->count == 0)
        pthread_cond_signal(&semap->nonzero);
    semap->count++;
    //sprintf("semap post = %d\n", semap->count);
    pthread_mutex_unlock(&semap->lock);
}
/*
 * -----------------------------------------------------------
 * Function     : semaphore_wait
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */

void semaphore_wait(semaphore_t *semap)
{
    pthread_mutex_lock(&semap->lock);
    while (semap->count == 0)
        pthread_cond_wait(&semap->nonzero, &semap->lock);
    semap->count--;
    //printf("semap wait = %d\n", semap->count);
    pthread_mutex_unlock(&semap->lock);
}
/*
 * -----------------------------------------------------------
 * Function     : semaphore_close
 * Description  : 
 * Input        : 
 *
 * Output       : 
 * -----------------------------------------------------------
 */

void semaphore_close(semaphore_t *semap)
{
    munmap((void *) semap, sizeof(semaphore_t));
}
