/*********************************************************************************
 * FILE: reader_writer.c                                                         *
 * AUTHOR: Christopher Hsu Chang                                                 *
 * STUDENT ID: 18821354                                                          *
 * UNIT: Operating Systems                                                       *
 * PURPOSE: File that does the reader and writer logic for the reader's writers  *
 *          problem                                                              *
 * REFERENCE: none                                                               *
 * REQUIRES: nothing                                                             *
 * LAST MOD: 7th May 2018                                                        *
 * DUE DATE: 7th May 2018                                                        *
 * ******************************************************************************/


#include "reader_writer.h"
/*******************************************************************************
 * FUNCTION: writer                                                             *
 * PARAMETERS: data_buffer, semaphores, t2, tempArray, pid, readcount, tempCount*
 *             writersPtr, trackerArray, numReaders                             *
 * RETURN: none                                                                 *
 * ASSERTION: The Writer function that each child process goes into.            *
 *            It syncronizes with the reader to check if a writer is allowed    *
 *            to write to the data_buffer, once all of the writing is done      *
 *            the writer writes out a log to a sim_out text file                *
 *******************************************************************************/

void writer(int* data_buffer, sem_t* semaphores, int* t2, int* tempArray, int* pid,
             int* readcount, int* tempCount, int* writersPtr, int* trackerArray, 
             int* numReaders)
{
    int datacount;

    
    
    
    datacount = 0;
    // To know if the writers have finished writing everything
    while( (*writersPtr) < (*tempCount) )
    {
        sem_wait(&semaphores[0]);       // Busy Waiting for readers to finish

        /* START WRITING */
        
        // If not zero don't write to the data buffer cuz readers need to read
        // (*writersPtr<*tempCount) ensures only one writer can enter the if statement
        // at a time
        if((trackerArray[*writersPtr%BUFFER_SIZE] == 0)&&((*writersPtr)<(*tempCount)))
        {
            /*Move value from temparray to the shared memory */
            data_buffer[(*writersPtr)%BUFFER_SIZE] = tempArray[(*writersPtr)];

            // Stored to let readers know how many readers need to read this element 
            // in data_buffer
            trackerArray[(*writersPtr)%BUFFER_SIZE] = (*numReaders);

            (*writersPtr)++;
            datacount++;
        }
                                            
        /* FINISH WRITING */
        
        sem_post(&semaphores[0]);       // Unlocks mutex lock so next 
                                        //writer can write or if a reader 
                                        //wants to read
                                        

        sleep(*t2);
    }
    
    // Lock used here so only one writer can write out at a time
    sem_wait(&semaphores[3]);
    writeOutFile(getpid(), datacount, "writer-%d has finished writing %d pieces of data to the data buffer\n");
    sem_post(&semaphores[3]);

    printf("---WRITER %d has printed %d elements\n", getpid(), datacount);
}

/*******************************************************************************
 * FUNCTION: reader                                                             *
 * PARAMETERS: data_buffer, semaphores, readcount, readersPtr, t1, writersPtr   *
 * , trackerArray, tempCount                                                    *
 * RETURN: none                                                                 *
 * ASSERTION: reader function for each child process. It syncronizes with the   *
 * writers to see if a reader is allowed to read the data_buffer. The main      *
 * difference being is that multiple readers are allowed to read the same data  *
 * buffer where as a writer is only allowed to write one at a time. Once the    *
 * simulation is finished the reader function calls the writeOutFile function   *
 * to write out/append to the end of the text file logging its history          *
 * *****************************************************************************/

void reader(int* data_buffer, sem_t* semaphores, int* readcount, int* readersPtr,
            int* t1, int* writersPtr, int* trackerArray, int* tempCount)
{
    int datacount;
    int j;
    int readerArray[*tempCount];


    /* This for loop is Just used for Debugging */
    for(int k = 0 ; k < *tempCount; k++)
    {
        readerArray[k] = 0;
    }

    j = 0;


    datacount = 0;
    // Reader should read the whole data_buffer
    while( (j < (*tempCount)) )
    {

        if(j < *writersPtr)
        {

            sem_wait(&semaphores[1]);   /* Ensure mutual exclusion on readcount */

            (*(readcount))++;
            
            if( (*(readcount)) == 1 )
            {
                /* rw_mutex to see if any writing is being performed */
                sem_wait(&semaphores[0]);
            }
            // Unlock so readers can read
            sem_post(&semaphores[1]);



            // READING IS PERFORMED HERE
            readerArray[j] = data_buffer[j%BUFFER_SIZE];

            sem_wait(&semaphores[2]);
            // Once read, decrement the number of readers left to read this value
            trackerArray[j%BUFFER_SIZE] = trackerArray[j%BUFFER_SIZE] - 1;
            
            sem_post(&semaphores[2]);

            j++;
            datacount++;
            
            sem_wait(&semaphores[3]);
            (*readersPtr)++;
            sem_post(&semaphores[3]);

            // Lock the readcount variable
            sem_wait(&semaphores[1]);
            (*(readcount))--;

            // When there are no more readers currently reading
            if( (*(readcount)) == 0)
            {
                // Unlock the rw_mutex so the writer may use it
                sem_post(&semaphores[0]);
            }
            sem_post(&semaphores[1]);   // Unlocks the mutex so you can read

            sleep(*t1);

        }
    }
    // TESTING PART OF THE ASSIGNMENT
    printf("Reader ID %d has completed reading :", getpid());
    printReaderArray(readerArray, tempCount);

    // Lock Writing out to file 
    sem_wait(&semaphores[3]);
    writeOutFile(getpid(), datacount, "reader-%d has finished reading %d pieces of data from the data_buffer\n");
    sem_post(&semaphores[3]);

    printf("---READER %d has printed %d elements\n", getpid(), datacount);
}


/*******************************************************************************
 * FUNCTION: printBuffer                                                        *
 * IMPORTS: data_buffer                                                         *
 * RETURN: none                                                                 *
 * ASSERTION: Prints out any of the buffers, it can do the data_buffer or the   *
 * array tracker                                                                *
 * *****************************************************************************/
void printBuffer(int* data_buffer)
{
    printf("[");
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        if(i == BUFFER_SIZE - 1)
        {
            printf("%d", data_buffer[i]);
        }
        else
        {

            printf("%d, ", data_buffer[i]);
        }
    }
    printf("]\n");
}


/*******************************************************************************
 * FUNCTION: printReaderArray                                                   *
 * IMPORTS: buffer                                                              *
 * RETURN: none                                                                 *
 * ASSERTION: Prints out any of the buffers, it can do the data_buffer or the   *
 * array tracker                                                                *
 * *****************************************************************************/
void printReaderArray(int* buffer, int* tempCount)
{
    printf("[");
    for(int i = 0; i < *tempCount; i++)
    {
        if(i == (*tempCount) - 1)
        {
            printf("%d", buffer[i]);
        }
        else
        {
            printf("%d, ", buffer[i]);
        }
    }
    printf("]\n");
}
