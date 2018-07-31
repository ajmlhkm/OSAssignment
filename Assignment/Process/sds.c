/*********************************************************************************
 * FILE: sds.c                                                                   *
 * AUTHOR: Christopher Hsu Chang                                                 *
 * STUDENT ID: 18821354                                                          *
 * UNIT: Operating Systems                                                       *
 * PURPOSE: The Main fo the program that does all of the initializing and clean  *
 *          up of all the processes                                              *
 * REFERENCE: none                                                               *
 * REQUIRES: nothing                                                             *
 * LAST MOD: 7th May 2018                                                        *
 * DUE DATE: 7th May 2018                                                        *
 * ******************************************************************************/


#include "sds.h"
int main(int argc, char* argv[])
{
    // FILE DESCRIPTORS
    int data_bufferFD, readersFD, writersFD, semaphoreFD, readcountFD;   // Used to access resources
    int trackerFD;           
    // Shared Memory Pointers
    int *data_bufferP, *readersP, *writersP, *readcountP;
    int *trackerP;      // Array in Shared Memory to keep track of who has read what
    // Struct Declaration
    CommandLineInfo standardIn;
    // Variables
    sem_t rw_mutex;    // Used so Readers and Writers don't access data at  same time
    sem_t mutex_lock;  /* Used to not allow separate readers to update the count at 
                          the same time */
    sem_t array_track_sem;      /* Used so when modifying the array tracker 
                                   only one reader at a time */
    sem_t write_out_sem;

    // Semaphores will be stored in an array 
    sem_t *semaphores;

    int pid;            // Process ID
    int tempArray[FILE_SIZE];
    int tempArrayCount;
    int numChildren;

    // Validate Command Line Arguments
    validateArgs(argc, argv);

    // Set the valid arguments into the struct
    standardIn.numReaders = atoi(argv[1]);
    standardIn.numWriters = atoi(argv[2]);
    standardIn.t1         = atoi(argv[3]);
    standardIn.t2         = atoi(argv[4]);


    // Initialize the Shared Memory
    initMemory(&data_bufferFD, &readersFD, &writersFD, &readcountFD, &semaphoreFD,
               &trackerFD);

    // Map the Memory
    mapMemory(&data_bufferFD, &readersFD, &writersFD, &readcountFD, &semaphoreFD,
              &trackerFD, &data_bufferP, &readersP, &writersP, 
              &readcountP, &semaphores, &trackerP, &standardIn);


    /* READER WRITER FUNCTIONS START :) */
    
    // Initialize Parameters
    *readcountP = 0;                  // Number of processes currently reading object
    // Initializes semaphores
    initSem(&rw_mutex, &mutex_lock, &array_track_sem, &write_out_sem);
    semaphores[0] = rw_mutex;       // Ensure mutual exclusion for readcount
    semaphores[1] = mutex_lock;     /* For writers critical section and used by 
                                       the first and last readers */
    semaphores[2] = array_track_sem;
    semaphores[3] = write_out_sem;      // Lock for writing out to file

    pid = 1;                        //Set pid to some positive value because the
    tempArrayCount = 0;
    *readersP = 0;
    *writersP = 0;
    numChildren = 0;



    readFile("shared_data.txt", tempArray, &tempArrayCount);

    // Intialize all the values inside the data buffer to -1
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        data_bufferP[i] = -1;
    }

    // Initialize all of the values of the tracker buffer to 0
    for(int j = 0; j < BUFFER_SIZE; j++)
    {
        trackerP[j] = 0;
    }


    instantiateReaders(&pid, &standardIn, data_bufferP, semaphores, readcountP, 
                       readersP, writersP, trackerP, &tempArrayCount);
    if(pid > 0)                             // Only the Parent should create writers
    {
        // Creates all of the writer processes
        instantiateWriters(data_bufferP, &pid, &standardIn, tempArray, 
                           &tempArrayCount, semaphores, readcountP, writersP, 
                           trackerP);  
    }

    if( pid > 0 )       // The Parent so he can wait for all his children
    {
        numChildren = (standardIn.numReaders) + (standardIn.numWriters);

        // For Each Child the parent has to wait for all the kids to end before 
        // the parent can finish the program
        for(int k = 0; k < numChildren; k++)
        {
            wait(NULL);
        }
        cleanMemory(data_bufferFD, readersFD, writersFD, semaphoreFD, readcountFD
                    , trackerFD, &data_bufferP, &readersP, &writersP, &readcountP,
                    &trackerP, &semaphores); 
    }
    

    return 0;
}

/****************************************************************************
 * FUNCTION: initMemory                                                     *
 * PARAMETERS: data_bufferFD (int*), readersFD (int*), writersFD (int*),    *
 *             readcountFD (int*), semaphoreFD (int*), trackerFD (int*)     *
 * RETURN: none                                                             *
 * ASSERTION: Creates the shared memory for the readers and writers to      *
 * access                                                                   *
 * *************************************************************************/
void initMemory(int* data_bufferFD, int* readersFD, int* writersFD, int* readcountFD
                , int* semaphoreFD, int* trackerFD)
{
    // Create Shared Memory Object
    *data_bufferFD = (int) shm_open("data_buffer", O_CREAT | O_RDWR, 0666);
    *readersFD = (int) shm_open("readers", O_CREAT | O_RDWR, 0666);
    *writersFD = (int) shm_open("writers", O_CREAT | O_RDWR, 0666);
    *readcountFD = (int) shm_open("readcount", O_CREAT | O_RDWR, 0666);
    *semaphoreFD = (int) shm_open("semaphore", O_CREAT | O_RDWR, 0666);
    *trackerFD = (int) shm_open("tracker", O_CREAT | O_RDWR, 0666);
    

    // Error Checking if Shared Memory was created properly
    if( (*data_bufferFD == -1) || (*readersFD == -1) || (*writersFD == -1) || 
        (*readcountFD == -1) || (*semaphoreFD == 0)  || (*trackerFD == -1))
    {
        perror("Error: ");
        fprintf(stderr, "Error in creating the shared memory objects\n");
        exit(1);
    }

    // Set size of shared memory
    if( ftruncate(*data_bufferFD, BUFFER_SIZE*sizeof(int) ) == -1 )
    {
        fprintf(stderr, "Error in setting size of data buffer\n");
    }
    if( ftruncate(*readersFD, sizeof(int)) == -1)
    {
        fprintf(stderr, "Error in setting size of readers buffer\n");
    }
    if( ftruncate(*writersFD, sizeof(int)) == -1)
    {
        fprintf(stderr, "Error in setting size of writers buffer\n");
    }
    if( ftruncate(*readcountFD, sizeof(int)) == -1)
    {
        fprintf(stderr, "Error in setting size of readcount buffer\n");
    }
    if( ftruncate(*semaphoreFD, sizeof(int)) == -1)
    {
        fprintf(stderr, "Error in setting size of semaphore buffer\n");
    }
    if( (ftruncate(*trackerFD, sizeof(int)) == -1) )
    {
        fprintf(stderr, "Error in setting size of tracker buffer\n");
    }

}


/****************************************************************************
 * FUNCTION: validateArgs                                                   *
 * PARAMETERS: argc (int), argv (char**)                                    *
 * RETURN: none                                                             *
 * ASSERTION: Validates the Command Line Parameters                         *
 * *************************************************************************/
void validateArgs(int argc, char* argv[])
{
    // Check the number of command line parameters
    if(argc != 5)
    {
        printf("Invalid Command Line Parameters!!\n");
        exit(1);
    }

    // Ensure there are a +ve number of readers/writers
    if( (atoi(argv[1]) <= 0) || (atoi(argv[2]) <= 0) ) 
    {
        printf("Readers/Writers cannot be 0 or negative number\n");
        exit(1);
    }

    // Ensure that the delay times are positive
    if( (atoi(argv[3]) < 0) || (atoi(argv[4]) < 0) )
    {
        printf("t1 and/or t2 sleep timers cannot be negative\n");
        exit(1);
    }
}

/****************************************************************************
 * FUNCTION: mapMemory                                                      *
 * PARAMETERS:  AS SHOW BELOW                                               *
 * RETURN: none                                                             *
 * ASSERTION: Uses mmap() to map the File Descriptors to the address space  *
 * *************************************************************************/
void mapMemory(int* data_bufferFD, int* readersFD, int* writersFD, int* readcountFD,
               int* semaphoreFD, int* trackerFD, int** data_bufferP
               ,int** readersP, int** writersP, int** readcountP, 
               sem_t** semaphores, int** trackerP, CommandLineInfo* standardIn)
{
    // Memory Mapping 
    *data_bufferP = (int*)mmap(NULL, BUFFER_SIZE*sizeof(int), PROT_READ | PROT_WRITE,
                                MAP_SHARED, *data_bufferFD, 0);
    *readersP =  (int*)mmap(NULL, standardIn->numReaders*sizeof(int), PROT_READ | 
                PROT_WRITE, MAP_SHARED, *readersFD, 0);
    *writersP = (int*)mmap(NULL, standardIn->numWriters*sizeof(int), PROT_READ |
                PROT_WRITE, MAP_SHARED, *writersFD, 0);
    *readcountP = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                  MAP_SHARED, *readcountFD, 0);
    *semaphores = mmap(NULL, 4*sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED
                  , *semaphoreFD, 0);
    *trackerP = (int*)mmap(NULL, BUFFER_SIZE*sizeof(int), PROT_READ | PROT_WRITE,
                MAP_SHARED, *trackerFD, 0);
}

/****************************************************************************
 * FUNCTION: initSem                                                        *
 * PARAMETERS: rw_mutex (sem_t**), mutex_lock (sem_t**), array_track        *
 *             (sem_t*), write_out_lock (sem_t)                             *
 * RETURN: none                                                             *
 * ASSERTION: Checks the semaphores then initializes them, sets the         *
 * readwrite mutex to 1 and the mutex lock to 1                             *
 * *************************************************************************/
void initSem(sem_t* rw_mutex, sem_t* mutex_lock, sem_t* array_track_sem, sem_t*
        write_out_lock)
{
    if( (sem_init(rw_mutex, 1, 1) == 1) || (sem_init(mutex_lock, 1, 1)) || (sem_init(
         array_track_sem, 1, 1)) || (sem_init(write_out_lock, 1, 1)) )
    {
        fprintf(stderr, "Error in initializing semaphores\n");
        exit(1);
    }
}

/****************************************************************************** *
 * FUNCTION: instantiateWriters                                                 *
 * PARAMETERS:                                                                  *
 * RETURN none:                                                                 *
 * ASSERTION: Creates the writers processes dependent on the command line       *
 *            arguments then each child process runs their respective writer    *
 *            functions                                                         *
 ******************************************************************************/
void instantiateWriters(int* data_buffer, int* pid, CommandLineInfo* standardIn, 
                        int* tempArray, int* tempArrayCount, sem_t* semaphores
                        , int* readcountP, int* writersP, int* trackerP)
{

    // Writer Process
    for(int i = 0; i < standardIn->numWriters; i++)
    {
        // pid is the Parent
        if( (*pid) > 0 )
        {
            (*pid) = fork();                       // Creates child process
        }
    }

    if( (*pid) == 0 )                     // Child Process
    {
        writer(data_buffer, semaphores, &(standardIn->t2), tempArray, pid, readcountP
               , tempArrayCount, writersP, trackerP, &(standardIn->numReaders));
    }

    if( (*pid) < 0 )                    // fork fails to create child process
    {
        fprintf(stderr, "Error: Unable to create child process, please run \"killall sds\"\n");
        exit(1);
    }
}

/*******************************************************************************
 * FUNCTION: instantiateReaders                                                 *
 * PARAMETERS: AS SHOW BELOW                                                    *
 * RETURN: none                                                                 *
 * ASSERTION: Creates the readers processes dependent on the command line       *
 *            arguments then each child process runs their respective reader    *
 *            functions                                                         *
 *******************************************************************************/
void instantiateReaders(int* pid, CommandLineInfo* standardIn, int* data_bufferP
                        , sem_t* semaphores, int* readcountP, int* readersP,
                        int* writersP, int* trackerP, int* tempArrayCount)
{

    // Readers Process
    for(int i = 0; i < standardIn->numReaders; i++)
    {
        // pid is the Parent
        if( (*pid) > 0 )
        {
            (*pid) = fork();                       // Creates child process
        }
    }

    if( (*pid) == 0 )                     // Child Process
    {
        reader(data_bufferP, semaphores, readcountP, readersP, &(standardIn->t1),
               writersP, trackerP, tempArrayCount);
    }

    if( (*pid) < 0 )                    // fork fails to create child process
    {
        fprintf(stderr, "Error: Unable to create child process, please run \"killall sds\"\n");
        exit(1);
    }
}

/*******************************************************************************
 * FUNCTION: cleanMemory                                                        *
 * PARAMETERS: data_bufferFD, readersFD, writersFD, semaphoreFD, readcountFD,   *
 *             trackerFD, data_bufferP, readersP, writeresP, readcountP,        *
 *             trackerP, semaphores,                                            *
 * RETURN: none                                                                 *
 * ASSERTION: Called once all all of the child processes are finished and then  *
 *            parent process does all of the tearing down of the data. Including*
 *            Destroying semaphores, Unlinking pointers, closing file descriptors*
 *            and Unmapping Memory                                              *
 *******************************************************************************/
void cleanMemory(int data_bufferFD, int readersFD, int writersFD, int semaphoreFD, 
                 int readcountFD, int trackerFD, int** data_bufferP, int** readersP, 
                 int** writersP, int** readcountP, int** trackerP, sem_t** semaphores)
{

    // Destroy Semaphores
    sem_destroy(&(*semaphores)[0]);
    sem_destroy(&(*semaphores)[1]);
    sem_destroy(&(*semaphores)[2]);
    sem_destroy(&(*semaphores)[3]);

    // Unlink the pointers
    shm_unlink("data_buffer");
    shm_unlink("readers");
    shm_unlink("writers");
    shm_unlink("readcount");
    shm_unlink("semaphore");
    shm_unlink("tracker");

    // Close File Descriptors
    close(data_bufferFD);
    close(readersFD);
    close(writersFD);
    close(semaphoreFD);
    close(readcountFD);
    close(trackerFD);

    // Unmap Memory
    munmap(*data_bufferP, sizeof(int)*BUFFER_SIZE);
    munmap(*readersP, sizeof(int));
    munmap(*writersP, sizeof(int));
    munmap(*readcountP, sizeof(int));
    munmap(*trackerP, sizeof(int)*BUFFER_SIZE);
    munmap(*semaphores, sizeof(int)*4);
}
