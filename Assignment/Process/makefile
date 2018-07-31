#********************************************************************************
# FILE: makefile                                                                *
# AUTHOR: Christopher Hsu Chang                                                 *
# STUDENT ID: 18821354                                                          *
# UNIT: Operating Systems                                                       *
# PURPOSE: MAKEFILE 									                        *
# REFERENCE: none                                                               *
# REQUIRES: nothing                                                             *
# LAST MOD: 7th May 2018                                                        *
# DUE DATE: 7th May 2018                                                        *
# ******************************************************************************#
#MAKEFILE

#Environment Variables

CC = gcc
CFLAGS = -Wall -pedantic -std=c99 -D_XOPEN_SOURCE=500 -D_SVID_SOURCE -lrt -pthread -g
OBJ = sds.o fileio.o reader_writer.o
EXEC = sds

$(EXEC) : $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(CFLAGS)

sds.o : sds.c sds.h fileio.h reader_writer.h
	$(CC) -c sds.c $(CFLAGS)

fileio.o : fileio.c fileio.h
	$(CC) -c fileio.c $(CFLAGS)

reader_writer.o : reader_writer.c reader_writer.h fileio.h
	$(CC) -c reader_writer.c $(CFLAGS)

clean:
	rm -f $(EXEC) sds.o fileio.o reader_writer.o sim_out

run:
	./$(EXEC) 2 4 1 2
