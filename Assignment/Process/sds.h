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

/* sds.h - Header File for sds.c */

#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

//#include "reader_writer.h"
#include "fileio.h"
#include "reader_writer.h"

// CONSTANTS
#define FILE_SIZE 100
#define FALSE 0
#define TRUE !FALSE


// STRUCTS
typedef struct
{
    int numReaders;
    int numWriters;
    int t1;    // value for the reader's sleep
    int t2;    // value for the writer's sleep
}CommandLineInfo;


// FUNCTION DECLARATIONS
void initMemory(int*, int*, int*, int*, int*, int*);
void validateArgs(int, char**);
void mapMemory(int*, int*, int*, int*, int*, int*, int**, int**, int**, int**, 
               sem_t**, int**, CommandLineInfo*);
void initSem(sem_t*, sem_t*, sem_t*, sem_t*);
void instantiateReaders(int*, CommandLineInfo*, int*, sem_t*, int*, int*, int*, int*, int*);
void instantiateWriters(int*, int*, CommandLineInfo*, int*, int*, sem_t*, int*, int*, int*);
void cleanMemory(int, int, int, int, int, int, int**, int**, int**, int**, int**, sem_t**);
