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
#include <string.h>
/*
 * -----------------------------------------------------------
 * MACRO (define) section
 * -----------------------------------------------------------
 */

#define     MAX_WORD_SIZE           20

/*
 * -----------------------------------------------------------
 * Type definition section
 * -----------------------------------------------------------
 */

typedef struct key_value
{
    char                word[MAX_WORD_SIZE];
    unsigned int        Count;
    struct key_value    *ptr_next;
}key_value_t;

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

static int reducer();

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

/*
 * -----------------------------------------------------------
 * Function     : reducer
 * Description  : Gets the word key_value pairs from mapper in the
 *                format (word,1). It counts the words and generates
 *                (word, count).
 * Input        : read from standard output
 *
 * Output       : prints to the screen
 * -----------------------------------------------------------
 */
static int reducer()
{
    char *input;
    char keyword[MAX_WORD_SIZE];
    key_value_t *ptrHead, *ptrCurrent, *ptrNew, *ptrIterate;
    /// The first word goes to head of the linked list
    if(read(0, keyword, MAX_WORD_SIZE) != 0)
    {
        input = strtok(keyword, ",()1\n");                          // Extract the word
        ptrHead = (key_value_t*) calloc (1,sizeof(key_value_t));    // Allocate Head of the linked list
        strcpy(ptrHead->word, input);                               // Copy the extracted word
        ptrHead->ptr_next = NULL;                                   // Initialize ptr_next to NULL
        ptrHead->Count = 1;                                         // Initialize the Count to 1
        ptrCurrent = ptrHead;                                       // Head becomes current ptr
    }
    else
    {
        /// When there is no input from the file
        printf("Error: No inputs\n");
        return 0;
    }

    /// Get next inputs from the file
    if (read(0, keyword, MAX_WORD_SIZE) != 0)
    {
        do
        {
            input = strtok(keyword, ",()1\n");

            if (ptrCurrent->word[0] == input[0])                    // Check if the current ptr first character is same
            {                                                       // as the input
                ptrIterate = ptrHead;

                do                                                  // Iterate to check whether there is a word in the linked list
                {                                                   // which matches the input and increase the count if match found
                    if(strcmp(ptrIterate->word, input) == 0)
                    {
                        ptrIterate->Count += 1;
                        break;
                    }
                    ptrIterate = ptrIterate->ptr_next;
                }
                while(ptrIterate != NULL);

                if(ptrIterate == NULL)                              // If no match is found in the linked list, allocate memory and create new node
                {
                    ptrNew = (key_value_t*) calloc (1,sizeof(key_value_t));
                    strcpy(ptrNew->word, input);
                    ptrNew->Count = 1;
                    ptrCurrent->ptr_next = ptrNew;
                    ptrNew->ptr_next = NULL;
                    ptrCurrent = ptrNew;
                }
            }
            else                                                    // If the input first character is not same as the current ptr, write all the
            {                                                       // words and count of each word and free the linked list memory.
                ptrCurrent = ptrHead;
                do
                {
                    printf("(%s,%d)\n", ptrCurrent->word, ptrCurrent->Count);
                    ptrIterate = ptrCurrent->ptr_next;
                    free(ptrCurrent);
                    ptrCurrent = ptrIterate;
                }
                while(ptrCurrent != NULL);

                ptrHead = (key_value_t*) calloc (1,sizeof(key_value_t));    // Allocate the new word which starts with the different character to the head
                strcpy(ptrHead->word, input);                               // of the linked list
                ptrHead->ptr_next = NULL;
                ptrHead->Count = 1;
                ptrCurrent = ptrHead;
            }
        }
        while(read(0, keyword, MAX_WORD_SIZE) != 0);

        ptrCurrent = ptrHead;
        do                                                                  // Write the last words list here.
        {
            printf("(%s,%d)\n", ptrCurrent->word, ptrCurrent->Count);
            ptrIterate = ptrCurrent->ptr_next;
            free(ptrCurrent);
            ptrCurrent = ptrIterate;
        }
        while(ptrCurrent != NULL);
        ptrHead = NULL;
    }
    else
    {
        /// Only one word is given as input
        printf("(%s,%d)\n", ptrCurrent->word, ptrCurrent->Count);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    reducer();
}

/*
 * -----------------------------------------------------------
 * End of File
 * -----------------------------------------------------------
 */
