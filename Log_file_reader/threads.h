#ifndef threads_h
#define threads_h

/* System libs */
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

/* Own libs */
#include "includes.h"

/* Block size to read = 8MB - optimal for current search functions*/
#define BLOCK_SIZE 4194304
/* Size of the biggest line for read - 8KB*/
#define BIGEST_LINE 8192
/* Max line size that i will give for one line - 1MB */
#define MAX_LINE 1048576

/* Writer flags */
#define W_START   0
#define W_FIRST   1
#define W_CURENT  2
#define W_LAST    3

/* Stop flags */
#define TH_ERROR        8
#define TH_FILE_END     4
#define TH_SEARCH_DONE  2
#define TH_MAX_LINES    1

/* Threads id for flag check */
#define READER   1
#define SEARCHER 2
#define WRITER   3


/* Struct for threads */
typedef struct common_data
{
    /* Queues for search and write */
    list_t *plSearchQueue, *plWrightQueue;
    /* Flag for stoping the process */
    int uiFlags;
    //int iFile_end, iMax_lines, Search_is_done, iError;
    struct input_t *pInputData;
    
} common_data_t;

/* A context for file reader */
typedef struct reader_context
{
    FILE *file;
    char *szLine, *Block;
    long lFileSize, lBytesToReadOnce;
    int  iCurent_LineSize;
} reader_t;

/* Writer thread context */
typedef struct writer_context
{
    FILE *file;
    char *szMask, *szSeparator;
    int iAmaunt, iFlag;
    node_t *pnTemp;

} writer_t;




/* ----------- Threads control functions-----------*/
/* Init common data structure */
common_data_t* threads_data_init(struct input_t *pInputData);

/* Try to init mutexes and semaphores */
int threads_sync_init();

/* Try to start threads */
int threads_start(common_data_t *pstThreadData);

/* Wait end of threads */
void threads_join();

/* destroy semaphores and mutexes */
void threads_sync_destroy();
/* --------------------END-----------------------*/





/* -----------Common threads functions-----------*/
/* Post the flag in common data */
void thread_flag_post(int *flag, int thread_id);

/* Check flags */
int thread_flag_check(int *uiFlag, int thread_id);

/* Push new node in queue */
int push_new_node_to_queue(node_t *pnTemp, list_t *plQueue,
                            pthread_mutex_t *mutex, sem_t *semaphore);

/* Get new node from queue */
node_t* get_node_from_queue(list_t *plQueue, pthread_mutex_t *mutex,
                                int* iFlag);
/* --------------------END-----------------------*/





/* Reader thread function*/
void* reader_thread(void *pThreadData);

    /* ----------Reader thread functions------------*/
    /* init file reader structure */
    reader_t* reader_t_init(char* file_path);

    /* Destroy file reader structure */
    void reader_t_destroy(reader_t* pReader_t);

    /* Read file from head */
    int read_file_head(common_data_t *pthData, reader_t* pReader_t);

    /* Read file from tail */
    int read_file_tail(common_data_t *pthData, reader_t* pReader_t);
    /* --------------------END-----------------------*/




/* Searcher thread function*/
void* search_thread(void *pThreadData);





/* writer thread function*/
void* writer_thread(void *pThreadData);
    /* ----------Writer thread functions------------*/
    /* init file writer structure */
    writer_t* writer_t_init(struct input_t *pInputData);

    /* Destroy file writer structure */
    void writer_t_destroy(writer_t* pWriter_t);

    /* switch for writing output strings to console or in file */
    int write_output_line(writer_t *pWriter_t);

    /* Printing the output lines in console */
    int show_output_on_console(writer_t *pWriter_t);

    /* Writing the output lines in file */
    int write_output_in_file(writer_t *pWriter_t);
    /* --------------------END----------------------*/

#endif /* threads_h */
