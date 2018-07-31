/*********************************************************************************
 * FILE: reader_writer.h                                                         *
 * AUTHOR: Christopher Hsu Chang                                                 *
 * STUDENT ID: 18821354                                                          *
 * UNIT: Operating Systems                                                       *
 * PURPOSE: Header file for reader_writer .c file                                *
 * REFERENCE: none                                                               *
 * REQUIRES: nothing                                                             *
 * LAST MOD: 7th May 2018                                                        *
 * DUE DATE: 7th May 2018                                                        *
 * ******************************************************************************/
/* Header file for reader_writer.c */
// HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#include "fileio.h"

// MACROS
#define BUFFER_SIZE 20
#define FALSE 0
#define TRUE !FALSE

//FORWARD DECLARATIONS
void reader(int*, sem_t*, int*, int*, int*, int*, int*, int*);
void writer(int*, sem_t*, int*, int*, int*, int*, int*, int*, int*, int*);
void printBuffer(int*);
void printReaderArray(int*, int*);
