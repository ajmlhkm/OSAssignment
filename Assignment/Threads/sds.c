/*********************************************************************************
 * FILE: sds.c                                                                   *
 * AUTHOR: Christopher Hsu Chang                                                 *
 * STUDENT ID: 18821354                                                          *
 * UNIT: Operating Systems                                                       *
 * PURPOSE: Does the readers writers problem given a number of readers and writer*
 *          -s by creating threads and running the reader() writer() functions   *
 * REFERENCE: none                                                               *
 * REQUIRES: nothing                                                             *
 * LAST MOD: 7th May 2018                                                        *
 * DUE DATE: 7th May 2018                                                        *
 * ******************************************************************************/

#include "sds.h"

SharedMem* sharedMem;
Pointers* ptr;

pthread_mutex_t mutex;          // Used for general locking and unlocking
pthread_cond_t cond;           // Has a condition with SIGNAL AND BROADCAST
pthread_mutex_t race;
pthread_mutex_t rw_mutex;
pthread_mutex_t read_mutex;
pthread_mutex_t writeFile_mutex;
pthread_mutex_t tracker_mutex;
int numR, numW, t1, t2;


int main(int argc, char* argv[])
{

    validateArgs(argc, argv);

    // Rename Command Line Arguments
    numR = atoi(argv[1]);
    numW = atoi(argv[2]);
    t1 = atoi(argv[3]);
    t2 = atoi(argv[4]);

    // Set Size of
    pthread_t readersArray[numR];     // Threads for readers
    pthread_t writersArray[numW];     // Threads for writers



    // Allocate Memory
    initMem(&sharedMem, &ptr);


    // Initialize Struct Variables
    sharedMem->letWriters = 0;
    sharedMem->writing = 0;
    sharedMem->reading = 0;
    sharedMem->readcount = 0;
    sharedMem->temp_array_count = 0;


    ptr->readPtr = 0;
    ptr->writePtr = 0; 


    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&race, NULL);
    pthread_mutex_init(&rw_mutex, NULL);
    pthread_mutex_init(&read_mutex, NULL);
    pthread_mutex_init(&writeFile_mutex, NULL);
    pthread_mutex_init(&tracker_mutex, NULL);
    pthread_cond_init(&cond, NULL);


    // Read the Input File
    readFile("shared_data.txt", &sharedMem);

    // Create the threads
    for(int i = 0; i < numR; i++)
    {
        pthread_create(&readersArray[i], NULL, reader, NULL);
    }
    for(int i = 0; i < numW; i++)
    {
        pthread_create(&writersArray[i], NULL, writer, NULL);
    }

    // Join the threads
    for(int i = 0; i < numR; i++)
    {
        pthread_join(readersArray[i], NULL);
    }

    for(int i = 0; i < numW; i++)
    {
        pthread_join(writersArray[i], NULL);
    }


    // Clean Memory
    cleanMemory();

    return 0;
}

/************************************************************************************
 * FUNCTION: validateArgs                                                           *
 * PARAMETERS: argc (int), argv (char**)                                            *
 * EXPORT: none                                                                     *
 * RETURN: none                                                                     *
 * ASSERTION: Validates command line arguments                                      *
 * *********************************************************************************/
void validateArgs(int argc, char* argv[])
{
    if(argc != 5)
    {
        printf("Invalid Command Line Arguments!!\n");
        exit(1);
    }

    // Ensure there are a +ve number of readers/writers
    if( (atoi(argv[1]) <= 0) || (atoi(argv[2]) <= 0) )
    {
        printf("Readers/Writers cannot be zero or negative number\n");
        exit(1);
    }

    // Ensure that the delay times are positive
    if( (atoi(argv[2]) < 0) || (atoi(argv[4]) < 0) )
    {
        printf("t1 and/or t2 sleep timers cannot be negative\n");
        exit(1);
    }
}

/************************************************************************************
 * FUNCTION: initMem                                                                *
 * PARAMETERS: sharedMem (SharedMem**), ptr (Pointers**)                            *
 * EXPORT: none                                                                     *
 * RETURN: none                                                                     *
 * ASSERTION: Initializes the memory by mallocing them                              *
 * *********************************************************************************/
void initMem(SharedMem** sharedMem, Pointers** ptr)
{
    (*sharedMem) = (SharedMem*)malloc(sizeof(SharedMem));
    (*sharedMem)->temp_array = (int*)calloc(FILE_SIZE, sizeof(int));
    (*sharedMem)->tracker_array = (int*)calloc(BUFFER_SIZE, sizeof(int));
    (*sharedMem)->data_buffer = (int*)calloc(BUFFER_SIZE, sizeof(int));

    (*ptr) = (Pointers*)malloc(sizeof(Pointers));
}

/************************************************************************************
 * FUNCTION: readFile                                                               *
 * PARAMETERS: fileName (char*), sharedMem (SharedMem**)                            *
 * EXPORT: none                                                                     *
 * RETURN: none                                                                     *
 * ASSERTION: Reads in the file and stores it into a temporary array                *
 * *********************************************************************************/
void readFile(char* fileName, SharedMem** sharedMem)
{
    FILE* fp = NULL;
    int value, i;

    fp = fopen(fileName, "r") ;

    if(fp == NULL)
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
            (*sharedMem)->temp_array[i] = value;
            i++;
            (*sharedMem)->temp_array_count++;
        }
        fclose(fp);
    }
}

/************************************************************************************
 * FUNCTION: reader                                                                 *
 * PARAMETERS: none                                                                 *
 * EXPORT: none                                                                     *
 * RETURN: (void*)                                                                  *
 * ASSERTION: The reader function for the child threads                             *
 * *********************************************************************************/
void* reader()
{
    int datacount;
    int j;
    int readerArray[sharedMem->temp_array_count];

    /* This for loop is used for debugging */
    for(int k = 0; k < sharedMem->temp_array_count; k++)
    {
        readerArray[k] = 0;
    }


    // DOES READING
    j = 0;
    datacount = 0;
    while(j < sharedMem->temp_array_count)
    {

        if(j < ptr->writePtr)
        {

            // Reader grabs the mutex lock
            pthread_mutex_lock(&mutex);
            
            // When a reader wants to read come in
            while(sharedMem->letWriters)
            {
                // Conditional Wait
                pthread_cond_wait(&cond, &mutex);    // Readers Busy Waiting
            }
            pthread_mutex_unlock(&mutex);

            // Protects the ReadCount variable
            pthread_mutex_lock(&read_mutex);
            sharedMem->readcount++;
            if(sharedMem->readcount == 1)
            {
                pthread_mutex_lock(&rw_mutex);
            }
            pthread_mutex_unlock(&read_mutex);

            // READING STARTS HERE

            // Decrement the Tracker Array Once read
            pthread_mutex_lock(&tracker_mutex);
            sharedMem->tracker_array[j%BUFFER_SIZE] = sharedMem->tracker_array[j%BUFFER_SIZE] - 1;
            pthread_mutex_unlock(&tracker_mutex);


            /* Reader Array is used for debugging */
            readerArray[j] = sharedMem->data_buffer[j%BUFFER_SIZE];

            datacount++;
            j++;
            
            
            // Lock The Temporary Array Count so only one reader can decrement it 
            // at a time
            pthread_mutex_lock(&race);
            ptr->readPtr++;
            pthread_mutex_unlock(&race);

            // Locks the readcount variable again!
            pthread_mutex_lock(&read_mutex);
            sharedMem->readcount--;
            if(sharedMem->readcount == 0)
            {
                // Unlock the rw mutex
                pthread_mutex_unlock(&rw_mutex);
            }
            pthread_mutex_unlock(&read_mutex);

            // Sleep
            sleep(t1);
        }
    }
    
    // Write to sim_out file
    pthread_mutex_lock(&writeFile_mutex);
    writeOutFile(pthread_self(), datacount, "reader-%d has finished reading %d pieces of data from the data_buffer\n");
    pthread_mutex_unlock(&writeFile_mutex);


    // TESTING PART FOR THE ASSIGNMENT
    printf("READER ID %lu has completed reading :", pthread_self());
    printReaderArray(readerArray);
    printf("---READER %lu has printed %d elements\n", pthread_self(), datacount);
    pthread_exit(NULL);
}


/************************************************************************************
 * FUNCTION: writer                                                                 *
 * IMPORTS: none                                                                    *
 * EXPORT: none                                                                     *
 * RETURN: (void*)                                                                  *
 * ASSERTION: writer function for the child threads                                 *
 * *********************************************************************************/
void* writer()
{
    int datacount;


    datacount = 0;
    while(ptr->writePtr < sharedMem->temp_array_count)
    {

        pthread_mutex_lock(&mutex);
        sharedMem->letWriters++;
        while(sharedMem->reading || sharedMem->writing)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        sharedMem->writing++;
        pthread_mutex_unlock(&mutex);

        // Busy Waiting for readers to finish
        pthread_mutex_lock(&rw_mutex);

        // START WRITING
        if(( (sharedMem->tracker_array[ptr->writePtr%BUFFER_SIZE] == 0 ) && 
            (ptr->writePtr < sharedMem->temp_array_count)))
        {
            sharedMem->data_buffer[ptr->writePtr%BUFFER_SIZE] = sharedMem->temp_array[ptr->writePtr];
            // Print the data buffer 

            // Store into the tracker array to let readers know how many readers 
            // need to read this element of the data_buffer
            /* numR is numReaders and numW is numWriters */
            sharedMem->tracker_array[(ptr->writePtr%BUFFER_SIZE)] = numR;

            ptr->writePtr++;
            datacount++;
        }

        // Unlock the writers Critical Section
        pthread_mutex_unlock(&rw_mutex);

        pthread_mutex_lock(&mutex);
        sharedMem->letWriters--;
        sharedMem->writing--;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);

        // Sleep after one thread has written to the buffer
        sleep(t2);
    }

    // Write Out to File
    pthread_mutex_lock(&writeFile_mutex);
    writeOutFile(pthread_self(), datacount, "writer-%d has finished writing %d pieces of data from the data_buffer\n");
    pthread_mutex_unlock(&writeFile_mutex);

    // TESTING PART OF THE ASSIGNMENT
    printf("---WRITER %lu has printed %d elements\n", pthread_self(), datacount);
    pthread_exit(NULL);
}


/************************************************************************************
 * FUNCTION: printBuffer                                                            *
 * IMPORTS: buffer (int*)                                                           *
 * EXPORT: none                                                                     *
 * RETURN: none                                                                     *
 * ASSERTION: Prints out the buffer for debugging                                   *
 * *********************************************************************************/
void printBuffer(int* buffer)
{
    printf("[");
    for(int i = 0; i <  BUFFER_SIZE; i++)
    {
        if(i == BUFFER_SIZE - 1)
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

/************************************************************************************
 * FUNCTION: printReaderArray                                                       *
 * IMPORTS: readerArray (int*)                                                      *
 * EXRPOT: none                                                                     *
 * RETURN: none                                                                     *
 * ASSERTION: Prints out the readerArray for debugging                              *
 * *********************************************************************************/
void printReaderArray(int* readerArray)
{
    printf("[");
    for(int i = 0; i < sharedMem->temp_array_count; i++)
    {
        if( i == sharedMem->temp_array_count - 1 )
        {
            printf("%d", readerArray[i]);
        }
        else
        {
            printf("%d ", readerArray[i]);
        }
    }
    printf("]\n");
}


/************************************************************************************
 * FUNCTION: writeOutFile                                                           *
 * IMPORTS: pid (pthread_t), datacount (int), message (char*)                       *
 * EXPORT: none                                                                     *
 * RETURN: none                                                                     *
 * ASSERTION: Writes out the result to the sim_out file for the reader and writer   *
 *            functions                                                             *
 ***********************************************************************************/
void writeOutFile(pthread_t pid, int datacount, char* message)
{
    FILE* outFile = NULL;

    // Open the file
    // Opening for appending to the end of a file
    outFile = fopen("sim_out", "a");

    if(outFile != NULL)
    {
        fprintf(outFile, message, pid, datacount);
        fclose(outFile);
    }
    else
    {
        perror("Error in opening the file for writing\n");
        exit(1);
    }
}


/************************************************************************************
 * FUNCTION: cleanMemory                                                            *
 * IMPORTS: none                                                                    *
 * EXPORT: none                                                                     *
 * RETURN: none                                                                     *
 * ASSERTION: Does all of the cleaning up such as destroying Mutexes and freeing up *
 *            any malloc'd memory when all of the child threads have finished and   *
 *            the program is about to end                                           *
 ***********************************************************************************/
void cleanMemory()
{
    
    // Destroy Mutexes
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&race);
    pthread_mutex_destroy(&rw_mutex);
    pthread_mutex_destroy(&read_mutex);
    pthread_mutex_destroy(&writeFile_mutex);
    pthread_mutex_destroy(&tracker_mutex);
    pthread_cond_destroy(&cond);

    free((sharedMem)->temp_array);
    free((sharedMem)->tracker_array);
    free((sharedMem)->data_buffer);
    free(sharedMem);
    free(ptr);


}
