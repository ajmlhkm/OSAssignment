/*********************************************************************************
 * FILE: fileio.h                                                                *
 * AUTHOR: Christopher Hsu Chang                                                 *
 * STUDENT ID: 18821354                                                          *
 * UNIT: Operating Systems                                                       *
 * PURPOSE: Header file for fileio.c                                             *
 * REFERENCE: none                                                               *
 * REQUIRES: nothing                                                             *
 * LAST MOD: 7th May 2018                                                        *
 * DUE DATE: 7th May 2018                                                        *
 * ******************************************************************************/

/* Header File for fileio.c */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// FUNCTION DECLARATIONS
void readFile(char*, int*, int*);
void writeOutFile(int, int, char*);
