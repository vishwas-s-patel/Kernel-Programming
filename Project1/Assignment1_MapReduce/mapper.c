/* -----------------------------------------------------------
 * File name   : mapper.c
 * Description : Gets the input from the file and for each word 
 *               in the file generates key value pairs in the 
 *               form (word,1). The output is written to standard
 *               output.
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
#include <unistd.h>
#include <sys/types.h>

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

static void mapper(char *inputFile);

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
 * Function     : mapper
 * Description  : Gets the input from the file and for each word 
 *                in the file generates key value pairs in the 
 *                form (word,1). The output is written to standard
 *                output.
 * Input        : char *inputFile   -> Input from the file 
 *
 * Output       : Standard Output
 * -----------------------------------------------------------
 */
static void mapper(char *inputFile)
{
    char *input;
    char keyword[MAX_WORD_SIZE], output[MAX_WORD_SIZE];
    FILE *fp1;

    fp1 = fopen(inputFile, "r+");

    // Get input from the file until the end of file
    while(fgets(keyword, MAX_WORD_SIZE, fp1) != NULL)
    {
        input = strtok(keyword, "\n");                  // Extract the word
        strcpy(output, "(");
        strcat(output, input);
        strcat(output, ",1)\n");
        
        write(STDOUT_FILENO, output, MAX_WORD_SIZE);    // Write to Standard Output  
    }

    fclose(fp1);
}

int main(int argc, char *argv[])
{
    mapper(argv[1]);
    return 0;
}

/*
 * -----------------------------------------------------------
 * End of File
 * -----------------------------------------------------------
 */
