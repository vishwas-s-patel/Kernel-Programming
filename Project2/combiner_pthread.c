/* -----------------------------------------------------------
 * File name   : combiner_pthread.c
 * Description : The updater, mapper, reducer and summarizer_word_count_writer
 *               threads are communicating as producer and consumer
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
#include <string.h>
#include <pthread.h>

/*
 * -----------------------------------------------------------
 * MACRO (define) section
 * -----------------------------------------------------------
 */
#define     MAX_WORD_SIZE           20
#define     MAX_BUF_SIZE            10
#define     MAX_REDUCER_BUF_SIZE    100
#define     TRUE                    1
#define     FALSE                   0
/*
 * -----------------------------------------------------------
 * Type definition section
 * -----------------------------------------------------------
 */

// Circular buffer for producer and consumer between each function
typedef struct circular_buffer
{
    char            buffer[MAX_BUF_SIZE][MAX_WORD_SIZE];
    int             num;
    int             N;
}circular_buffer_t;

typedef struct redr_wrd_cnt
{
    char  redr_words[MAX_WORD_SIZE+5];
    int   count;
}redr_word_count_t;

typedef struct alpha_count
{
    char                alphabet;
    unsigned int        Count;
}alpha_count_t;

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
void *updater(void *id);
void *mapper(void *id);
void *reducer(void *id);
void *summarizer_word_count_writer(void *id);
void  init_ftn();

/*
 * -----------------------------------------------------------
 * Global data section
 * -----------------------------------------------------------
 */

FILE *fp;
FILE *fp1;
FILE *fp2;

pthread_mutex_t     updt_buf_mutex;
pthread_cond_t      updt_buf_full;
pthread_cond_t      updt_buf_empty;

pthread_mutex_t     mapr_buf_mutex;
pthread_cond_t      mapr_buf_full;
pthread_cond_t      mapr_buf_empty;

pthread_mutex_t     redr_buf_mutex;
pthread_cond_t      redr_buf_full;
pthread_cond_t      redr_buf_empty;

circular_buffer_t   updt_buffer;
circular_buffer_t   mapr_buffer;
circular_buffer_t   redr_buffer;

redr_word_count_t   redr_word_count[MAX_REDUCER_BUF_SIZE];

alpha_count_t       curr_alphabet;

int                 updt_finish_flag     = 0;
int                 mapr_finish_flag     = 0;
int                 redr_finish_flag     = 0;

int                 no_rem_reducer_words = 0;
int                 i_for_reducer        = 0;
int                 loop;
int                 loop_summ;

int                 redr_pin;
int                 redr_pout;
int                 mapr_pin;
int                 mapr_pout;
int                 pin;
int                 pout;
int                 last_word_flag = 1;

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

/*
 * -----------------------------------------------------------
 * Function     : updater
 * Description  : Reads from the file and updates the updater pool
 * Input        : thread_id
 *
 * Output       : The output is written to the buffe
 * -----------------------------------------------------------
 */
void *updater(void *id)
{
    long thread_id = (long)id;
    char *input;

    char keyword[MAX_WORD_SIZE];

    printf("thread updater is starting\n");
    while(1)
    {
        if(fgets(keyword, MAX_WORD_SIZE, fp) != NULL)
        {
            input = strtok(keyword, "\n");

            pthread_mutex_lock(&updt_buf_mutex);

            while(updt_buffer.num == updt_buffer.N)
            {
                pthread_cond_wait(&updt_buf_full, &updt_buf_mutex);
            }

            strcpy(updt_buffer.buffer[pin], input);             // Copy to updater pool

            pin = (pin + 1) % updt_buffer.N;                    // updater circular buffer
            updt_buffer.num++;

            if(updt_buffer.num == 1)
            {
                pthread_cond_broadcast(&updt_buf_empty);
            }

            pthread_mutex_unlock(&updt_buf_mutex);
        }
        else
        {   // Updater should not finish until all the threads in mapper finishes reading from buffer
            if(updt_buffer.num >= 1)
            {
                pthread_cond_broadcast(&updt_buf_empty);
            }
            else
            {
                break;
            }
        }
    }
    updt_finish_flag = 1;

    pthread_cond_broadcast(&updt_buf_empty);

    pthread_exit(NULL);
}

/*
 * -----------------------------------------------------------
 * Function     : mapper
 * Description  : mapper reads words from the updater pool and generates
 *                (word, 1) and write into mapper pool
 * Input        : thread_id
 *
 * Output       : Output is written to the mapper pool
 * -----------------------------------------------------------
 */
void *mapper(void *id)
{
    long    thread_id = (long) id;
    struct  node *myNode;
    char    in_word[MAX_WORD_SIZE];
    char    out_word[MAX_WORD_SIZE];
    int     i =0;

    printf("Thread mapper %ld starting \n", thread_id);
    while(!updt_finish_flag)
    {
        pthread_mutex_lock(&updt_buf_mutex);

        while(updt_buffer.num == 0 && updt_finish_flag == 0)
        {
            //printf("map cond wait for updater\n");
            pthread_cond_wait(&updt_buf_empty, &updt_buf_mutex);
        }

        if (!updt_finish_flag)
        {
            strcpy(in_word, updt_buffer.buffer[pout]);              // Read from updater pool

            //------- mapper logic starts---------------
            memset(out_word, 0, MAX_WORD_SIZE);
            strcpy(out_word, "(");
            strncat(out_word, in_word, MAX_WORD_SIZE);
            strncat(out_word, ",1)\n", 5);
            //------- mapper logic ends ----------------

            /////////////////////////////////////
            if (!mapr_finish_flag)
            {
                pthread_mutex_lock(&mapr_buf_mutex);

                while(mapr_buffer.num == mapr_buffer.N)
                {
                    pthread_cond_wait(&mapr_buf_full, &mapr_buf_mutex);
                }

                strcpy(mapr_buffer.buffer[mapr_pin], out_word);      // Write to mapper pool
                mapr_pin = (mapr_pin + 1) % mapr_buffer.N;           // mapper circular buffer
                mapr_buffer.num++;

                if(mapr_buffer.num == 1)
                {
                    pthread_cond_broadcast(&mapr_buf_empty);
                }

                pthread_mutex_unlock(&mapr_buf_mutex);
            }
            else
            {
                // mapper must not terminate until all the words are read from the circular buffer
                if(mapr_buffer.num >= 1)
                {
                    pthread_cond_broadcast(&mapr_buf_empty);
                }
                else
                {
                    break;
                }
            }
            /////////////////////////////////////

            pout = (pout + 1) % updt_buffer.N;                      // updater circular buffer
            updt_buffer.num--;

            if (updt_buffer.num == updt_buffer.N -1)
                pthread_cond_broadcast(&updt_buf_full);

        }
        pthread_mutex_unlock(&updt_buf_mutex);
    }

    mapr_finish_flag = 1;

    pthread_cond_broadcast(&mapr_buf_empty);

    printf("mapper thread %ld is terminating\n", thread_id);

    pthread_exit(NULL);
}

/*
 * -----------------------------------------------------------
 * Function     : reducer
 * Description  : reads the input (word,1)from mapper pool and reduces
 *                into (word,count) and writes into reducer pool
 * Input        : thread_id
 *
 * Output       : writes into reducer pool
 * -----------------------------------------------------------
 */
void *reducer(void *id)
{
    long thread_id = (long) id;
    char *input;
    char in_word[MAX_WORD_SIZE+4];
    char out_word[MAX_WORD_SIZE+5];
    char temp[10];
    int  i;
    int  redr_loop   = 0;
    int  i_redr_loop = 0;

    printf("Thread reducer %ld starting \n", thread_id);
    while(1)
    {
        if(!mapr_finish_flag)
        {
            pthread_mutex_lock(&mapr_buf_mutex);

            while(mapr_buffer.num == 0 && mapr_finish_flag == 0)
            {
                pthread_cond_wait(&mapr_buf_empty, &mapr_buf_mutex);
            }

            if(!mapr_finish_flag)
            {
                strcpy(in_word, mapr_buffer.buffer[mapr_pout]);             // Read from updater pool

                //////////////////////////////////////////////////
                //-------- reducer logic starts -------------------
                input = strtok(in_word, ",()1\n");
                if (loop == 0)
                {
                    strcpy(redr_word_count[loop].redr_words, input);
                    redr_word_count[loop].count = 1;
                    loop++;
                    i_redr_loop = 0;
                    redr_loop = loop;
                    no_rem_reducer_words = loop;
                }
                else if(redr_word_count[0].redr_words[0] == input[0])
                {
                    for(i=0; i<loop; i++)
                    {
                        if(strcmp(redr_word_count[i].redr_words, input) == 0)
                        {
                            redr_word_count[i].count += 1;
                            i_redr_loop = 0;
                            redr_loop = loop;
                            no_rem_reducer_words = loop;
                            break;
                        }
                    }
                    if(i == loop)
                    {
                        strcpy(redr_word_count[loop].redr_words, input);
                        redr_word_count[loop].count = 1;
                        loop++;
                    }
                    i_redr_loop = 0;
                    redr_loop = loop;
                    no_rem_reducer_words = loop;
                }
                else
                {
                    if(!redr_finish_flag)
                    {
                        ////////////////////////////////////////////////////

                        pthread_mutex_lock(&redr_buf_mutex);
                        while(loop > 0)
                        {

                            memset(out_word, 0, MAX_WORD_SIZE+5);
                            strcpy(out_word, "(");
                            strcat(out_word, redr_word_count[i_redr_loop].redr_words);
                            strcat(out_word, ",");
                            sprintf(out_word, "%s%d)\n", out_word, redr_word_count[i_redr_loop].count);

                            fprintf(fp1, "%s", out_word);

                            while(redr_buffer.num == redr_buffer.N)
                            {
                                pthread_cond_wait(&redr_buf_full, &redr_buf_mutex);
                            }
                            strcpy(redr_buffer.buffer[redr_pin], out_word);             // write into reducer pool
                            redr_pin = (redr_pin + 1) % redr_buffer.N;                  // reducer circular buffer
                            redr_buffer.num++;

                            if(redr_buffer.num == 1)
                            {
                                pthread_cond_broadcast(&redr_buf_empty);
                            }
                            loop--;
                            i_redr_loop++;
                        }

                        mapr_pout = ( mapr_pout + MAX_BUF_SIZE  - 1) % mapr_buffer.N;
                        mapr_buffer.num++;
                        no_rem_reducer_words = loop;
                        pthread_mutex_unlock(&redr_buf_mutex);

                        ////////////////////////////////////////////////////

                    }
                }

                //-------- reducer logic ends ----------------------
                ////////////////////////////////////////////////////

                mapr_pout = (mapr_pout + 1) % mapr_buffer.N;                    // mapper circular buffer
                mapr_buffer.num--;

                if (mapr_buffer.num == mapr_buffer.N -1)
                {
                    pthread_cond_broadcast(&mapr_buf_full);
                }
            }
            else
            {
                i_for_reducer=0;
                while(no_rem_reducer_words>0)
                {
                    memset(out_word, 0, MAX_WORD_SIZE+5);
                    strcpy(out_word, "(");
                    strcat(out_word, redr_word_count[i_for_reducer].redr_words);
                    strcat(out_word, ",");
                    sprintf(out_word, "%s%d)\n", out_word, redr_word_count[i_for_reducer].count);

                    ////////////////////////////////////////////////////

                    pthread_mutex_lock(&redr_buf_mutex);

                    while(redr_buffer.num == redr_buffer.N)
                    {
                        pthread_cond_wait(&redr_buf_full, &redr_buf_mutex);
                    }
                    strcpy(redr_buffer.buffer[redr_pin], out_word);
                    redr_pin = (redr_pin + 1) % redr_buffer.N;
                    redr_buffer.num++;

                    if(redr_buffer.num == 1)
                    {
                        pthread_cond_broadcast(&redr_buf_empty);
                    }

                    pthread_mutex_unlock(&redr_buf_mutex);

                    ///////////////////////////////////////////////////////

                    no_rem_reducer_words--;
                    i_for_reducer++;
                }
            }

            pthread_mutex_unlock(&mapr_buf_mutex);
        }
        else
        {
            // Reducer must not terminate until the reducer circular buffer is completely read
            if(redr_buffer.num > 0)
                pthread_cond_broadcast(&redr_buf_empty);
            else
                break;
        }
    }
    redr_finish_flag = 1;

    pthread_cond_broadcast(&redr_buf_empty);


    printf("reducer thread %ld is terminating\n", thread_id);

    pthread_exit(NULL);

}

/*
 * -----------------------------------------------------------
 * Function     : summarizer_word_count_writer
 * Description  :
 * Input        :
 *
 * Output       :
 * -----------------------------------------------------------
 */
void *summarizer_word_count_writer(void *id)
{
    long thread_id = (long)id;
    char *input, *in_count;
    char in_word[MAX_WORD_SIZE+4];

    printf("Summarizer Thread %ld starting \n", thread_id);

    while(1)
    {
        if(!redr_finish_flag || redr_buffer.num > 0)
        {
            pthread_mutex_lock(&redr_buf_mutex);

            while(redr_buffer.num == 0 && redr_finish_flag == 0)
            {
                pthread_cond_wait(&redr_buf_empty, &redr_buf_mutex);
            }

            if(!redr_finish_flag)
            {

                strcpy(in_word, redr_buffer.buffer[redr_pout]);

                // --------- summarizer_word_count_writer logic begins -------
                input = strtok(in_word, ",()\n");
                in_count = strtok(NULL, ",()\n");

                if (loop_summ == 0)
                {
                    curr_alphabet.Count = atoi(in_count);
                    curr_alphabet.alphabet = input[0];
                    loop_summ++;

                }
                else if(input[0] == curr_alphabet.alphabet)
                {
                    curr_alphabet.Count+= atoi(in_count);

                }
                else
                {
                    //printf("(%c,%d)\n", curr_alphabet.alphabet, curr_alphabet.Count);
                    fprintf(fp2, "(%c,%d)\n", curr_alphabet.alphabet, curr_alphabet.Count);

                    curr_alphabet.alphabet = input[0];
                    curr_alphabet.Count = atoi(in_count);

                    loop_summ = 1;
                }
                //--------- summarizer_word_count_writer logic ends ---------------

                redr_pout = (redr_pout + 1) % redr_buffer.N;
                redr_buffer.num--;

                if(redr_buffer.num == redr_buffer.N - 1)
                {
                    pthread_cond_broadcast(&redr_buf_full);
                }
            }
            pthread_mutex_unlock(&redr_buf_mutex);

        }
        else
        {
            // The last word
            //printf("(%c,%d)\n", curr_alphabet.alphabet, curr_alphabet.Count);
            if(last_word_flag == 1)
            {
                last_word_flag = 0;
                fprintf(fp2, "(%c,%d)\n", curr_alphabet.alphabet, curr_alphabet.Count);
            }

            break;
        }
    }
    printf("summarizer_word_count_writer thread %ld is terminating\n",thread_id);
    pthread_exit(NULL);

}

/*
 * -----------------------------------------------------------
 * Function     :
 * Description  :
 * Input        :
 *
 * Output       :
 * -----------------------------------------------------------
 */
void init_ftn()
{
    updt_buffer.num  = 0;
    mapr_buffer.num  = 0;
    redr_buffer.num  = 0;
    updt_buffer.N    = MAX_BUF_SIZE;
    mapr_buffer.N    = MAX_BUF_SIZE;
    redr_buffer.N    = MAX_BUF_SIZE;

    loop             = 0;
    redr_pin         = 0;
    redr_pin         = 0;
    redr_pout        = 0;
    mapr_pin         = 0;
    mapr_pout        = 0;
    pin              = 0;
    pout             = 0;
    loop_summ        = 0;
    redr_finish_flag = 0;
}

int main(int argc, char *argv[])
{
    int no_mapr_thrds;
    int no_redr_thrds;
    int no_sumr_thrds;

    int i, j, k;
    long id =1;

    char *inputfile;

    inputfile = argv[1];

    no_mapr_thrds = atoi(argv[2]);
    no_redr_thrds = atoi(argv[3]);
    no_sumr_thrds = atoi(argv[4]);

    long mapr_thrd_id[no_mapr_thrds];
    long redr_thrd_id[no_redr_thrds];
    long sumr_thrd_id[no_sumr_thrds];

    pthread_t updt_thrd;
    pthread_t mapr_thrds[no_mapr_thrds];
    pthread_t redr_thrds[no_redr_thrds];
    pthread_t sumr_thrds[no_sumr_thrds];

    //--------------------------------------------------------------------------

    pthread_mutex_init(&updt_buf_mutex, NULL);
    pthread_cond_init (&updt_buf_empty, NULL);
    pthread_cond_init (&updt_buf_full, NULL);

    pthread_mutex_init(&mapr_buf_mutex, NULL);
    pthread_cond_init (&mapr_buf_empty, NULL);
    pthread_cond_init (&mapr_buf_full, NULL);

    pthread_mutex_init(&redr_buf_mutex, NULL);
    pthread_cond_init (&redr_buf_empty, NULL);
    pthread_cond_init (&redr_buf_full, NULL);

    init_ftn();

    fp = fopen(inputfile, "r+");
    fp1 = fopen("wordCount.txt","w");
    fp2 = fopen("letterCount.txt","w");

    //---------------------------------------------------------------------------

    for (i = 0; i < no_sumr_thrds; i++)
    {
        printf("creating summarizer_word_count_writer thread = %d\n", i);
        sumr_thrd_id[i] = i;
        pthread_create(&sumr_thrds[i], NULL, summarizer_word_count_writer, (void *)sumr_thrd_id[i]);
    }

    for (i = 0; i < no_redr_thrds; i++)
    {
        printf("creating reducer thread = %d\n", i);
        redr_thrd_id[i] = i;
        pthread_create(&redr_thrds[i], NULL, reducer, (void *)redr_thrd_id[i]);
    }

    for (i = 0; i < no_mapr_thrds; i++)
    {
        printf("creating mapper thread = %d\n", i);
        mapr_thrd_id[i] = i;
        pthread_create(&mapr_thrds[i], NULL, mapper, (void *)mapr_thrd_id[i]);
    }

    pthread_create(&updt_thrd, NULL, updater, (void *)id);

    //--------------------------------------------------------------------------

    pthread_join(updt_thrd, NULL);
    printf("Exiting updater\n");

    for (i = 0; i < no_mapr_thrds; i++)
    {   printf("Exiting mapper = %d\n",i);
        pthread_join(mapr_thrds[i], NULL);
    }

    for (i = 0; i < no_redr_thrds; i++)
    {   printf("Exiting reducer = %d\n",i);
        pthread_join(redr_thrds[i], NULL);
    }

    for (i = 0; i < no_sumr_thrds; i++)
    {
        printf("Exiting summarizer_word_count_writer = %d\n",i);
        pthread_join(sumr_thrds[i], NULL);
    }

    //---------------------------------------------------------------------------

    fclose(fp);
    fclose(fp1);
    fclose(fp2);

    pthread_mutex_destroy(&updt_buf_mutex);
    pthread_cond_destroy(&updt_buf_empty);
    pthread_cond_destroy(&updt_buf_full);

    pthread_mutex_destroy(&mapr_buf_mutex);
    pthread_cond_destroy(&mapr_buf_empty);
    pthread_cond_destroy(&mapr_buf_full);

    pthread_mutex_destroy(&redr_buf_mutex);
    pthread_cond_destroy(&redr_buf_empty);
    pthread_cond_destroy(&redr_buf_full);

    printf("Exiting main\n");
    pthread_exit(NULL);

}
