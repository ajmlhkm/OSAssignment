/*********************************************************************************
 * FILE: sds.h                                                                   *
 * AUTHOR: Christopher Hsu Chang                                                 *
 * STUDENT ID: 18821354                                                          *
 * UNIT: Operating Systems                                                       *
 * PURPOSE: Header file for sds.c                                                *
 * REFERENCE: none                                                               *
 * REQUIRES: nothing                                                             *
 * LAST MOD: 7th May 2018                                                        *
 * DUE DATE: 7th May 2018                                                        *
 * ******************************************************************************/

/* Header File for sds.c */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

// CONSTANTS
#define BUFFER_SIZE 20
#define FILE_SIZE 100


// STRUCTS
typedef struct
{
   int* tracker_array;
   int  temp_array_count;
   int* temp_array;
   int* data_buffer;    //  Your Data Buffer
   int  letWriters;     //  Used to let a reader know that a writer wants to come in
   int  writing;        //  Tells you number of writers writing in critical section 
   int  reading;        //  Shows how many readers in CT
   int readcount;

}SharedMem;

typedef struct
{
    int readPtr;
    int writePtr;
}Pointers;


// FUNCTION DECLARATIONS
void validateArgs(int, char**);
void initMem(SharedMem**, Pointers**);
void readFile(char*, SharedMem**);
void* reader();
void* writer();
void printBuffer(int*);
void printReaderArray(int*);
void writeOutFile(pthread_t, int, char*);
void cleanMemory();
