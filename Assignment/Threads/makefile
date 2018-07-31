#********************************************************************************
 # FILE: makefile                                                                *
 # AUTHOR: Christopher Hsu Chang                                                 *
 # STUDENT ID: 18821354                                                          *
 # UNIT: Operating Systems                                                       *
 # PURPOSE: Makefile for threads 						                         *
 # REFERENCE: none                                                               *
 # REQUIRES: nothing                                                             *
 # LAST MOD: 7th May 2018                                                        *
 # DUE DATE: 7th May 2018                                                        *
 # ******************************************************************************/

# Environment Variables
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -pthread -g
OBJ = sds.o
EXEC = sds

$(EXEC) : $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(CFLAGS)

sds.o : sds.c sds.h 
	$(CC) -c sds.c $(CFLAGS)

clean :
	rm -f $(EXEC) $(OBJ) sim_out

run:
	./$(EXEC) 2 5 0 1
