#ifndef threads_h
#define threads_h

/* System libs */
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

/* Own libs */
#include "list_t.h"
#include "includes.h"


/* Block size to read = 4MB - optimal for current search functions*/
#define BLOCK_SIZE 4194304

/* Size of the biggest line for read - 8KB*/
#define BIGEST_LINE 8192

/* Max line size that i will give - 1MB */
#define MAX_LINE 1048576

/* Writer flags */
#define W_START   0
#define W_FIRST   1
#define W_CURENT  2
#define W_LAST    3

/* Stop flags */
#define TH_ERROR        0x08
#define TH_FILE_END     0x04
#define TH_SEARCH_DONE  0x02
#define TH_MAX_LINES    0x01

/* Threads id for flag check */
#define READER   1
#define SEARCHER 2
#define WRITER   3


/* Struct for threads */
typedef struct comon_data
{
    /* Queues for search and write */
    list_t *plSearchQueue, *plWrightQueue;
    /* Flag for stoping the process */
    int uiFlags;
    //int iFile_end, iMax_lines, Search_is_done, iError;
    struct input_t *pInputData;
    
} comon_data_t;

/* Tread Functions: */
/* Function for file reader thread */
void* reader_thread(void *pThreadData);

/* Function for searching thread */
void* search_thread(void *pThreadData);

/* Function for writer thread */
void* writer_thread(void *pThreadData);

/* Init common data structure */
comon_data_t* threads_data_init();

/* Init semaphores, mutexes and common data structure*/
int threads_init();

/* Start threads */
int threads_start(comon_data_t *stThreadData);

/* Wait threads to stop */
int thread_stop_wait();

/* Destroy semaphores and mutext */
void threads_destroy(comon_data_t *stThreadData);

/* Push new node in queue */
int push_new_node_to_queue(node_t *pnTemp, list_t *plQueue,
                           pthread_mutex_t *mutex, sem_t *semaphore);

/* Get new node from queue */
node_t* get_node_from_queue(list_t *plQueue, pthread_mutex_t *mutex,int* iFlag);

/* Post the flag in common data */
void thread_flag_post(int *flag, int thread_id);

/* Check flags */
int thread_flag_check(int *uiFlag, int thread_id);

/* Check the amaunt of lines in queue and sleep*/
void counter_check(int *iCounter);

/* switch for writing output strings to console or in file */
int write_output_line(char *szLine, int *iFlag, int *iAmount, char* szOptLine, FILE *optFile);

/* Printing the output lines in console */
int show_output_on_console(char *szLine, int *iFlag, int *iAmount, char* szOptLine);

/* Writing the output lines in file */
int write_output_in_file(char *szLine, int *iFlag, int *iAmount, char* szOptLine, FILE *optFile);



#endif /* threads_h */
