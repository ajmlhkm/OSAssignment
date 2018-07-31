/*********************************************************************************
 * FILE: fileio.c                                                                *
 * AUTHOR: Christopher Hsu Chang                                                 *
 * STUDENT ID: 18821354                                                          *
 * UNIT: Operating Systems                                                       *
 * PURPOSE: File Reading file that handles input and output                      *
 * REFERENCE: none                                                               *
 * REQUIRES: nothing                                                             *
 * LAST MOD: 7th May 2018                                                        *
 * DUE DATE: 7th May 2018                                                        *
 * ******************************************************************************/


#include "fileio.h"
/*******************************************************************************
 * FUNCTION: readFile                                                           *
 * PARAMETERS: fileName, tempArray, count                                       *
 * RETURN: none                                                                 *
 * ASSERTION: Reads in the input file shared_data.txt to be read into a         *
 * temporary array in which serves as the basis for the readers and writers to  *
 * read and write from an external databuffer                                   *
 * *****************************************************************************/
void readFile(char* fileName, int* tempArray, int* count)
{
    FILE* fp = NULL;
    int value, i;

    fp = fopen(fileName, "r");
    
    if(fp == NULL)                      // If you can't open the file
    {
        perror("Error, can not open the file\n");
        exit(1);
    }
    else
    {
        i = 0;
        while(!(feof(fp)))
        {
            if(fscanf(fp, "%d ", &value) != 1)
            {
                fprintf(stderr, "Error in formatting in the shared_data file\n");
                exit(1);
            }
            tempArray[i] = value;
            i++;
            (*count)++;
        }
        fclose(fp);
    }
}
/*******************************************************************************
 * FUNCTION: writeOutFile                                                       *
 * PARAMETERS: pid, count, message                                              *
 * RETURN: none                                                                 *
 * ASSERTION: When the simulation ends, the reader or the writer will append    *
 * the information (processID, local count) to the log which states how much    *
 * reading/writing each child process has done                                  *
 * *****************************************************************************/

void writeOutFile(int pid, int count, char* message)
{
    FILE* outFile = NULL;
    

    // Open the file
    // Open for appending to end of file
    outFile = fopen("sim_out", "a");
    
    if(outFile != NULL)
    {

        fprintf(outFile, message, pid, count);
        fclose(outFile);

    }
    else
    {
        perror("Error in opening the file\n");
        exit(1);
    }
}
