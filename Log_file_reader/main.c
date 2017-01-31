/* Include system librarys */
#include <pthread.h>

/* Include own librarys */
#include "list_t.h"
#include "usfull_utilities.h"

/* ./execute -f-file_path -m-mask -l-max_lines -t-scan_tail -s-separator */

/* Block size to read = 4MB */
#define BLOCK_SIZE 4194304

/* Codes to end program */
#define FILE_END    10
#define MAX_LINES   20

/* The answer to main question */
#define STOP        42

/* Global mutexes: 1 - read->search mutex, 2 - search->write mutex, 3 - common parameters mutex  */
pthread_mutex_t mutex_RS, mutex_SW, mutex_CP;

/* Input parameters */
const char *szMask, *szFilePath, *szSeparator;
int iScanTail, iMaxLines;

/* Struct for threads */
typedef struct comon_data
{
    
    /* Queues for search and write */
    list_t *plSearchQueue, *plWrightQueue;
    /* Flags for stoping the process */
    int iFile_end, iMax_lines;
    /* Flags that there is still something to search or write */
    int write, search;
    
} comon_data_t;


/* Search for mask in string, Return 0 if finded*/
int SearchForMaskInString(char * szString, regex_t *preg)
{
    return regexec(preg, szString, 0, NULL, 0);
}


/* Tread Functions: */
/* Function for reader thread */
void* reader_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    node_t *pnTemp = NULL;
    int stop = 0, file_end = 0;
    
    return NULL;
}
/* Function for searching thread */
void* search_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    node_t *pnTemp = NULL;
    regex_t preg;
    int stop = 0, search = 0, file_end = 0;
    
    /* Compiling regular expression */
    if ((regcomp(&preg, szMask, 0))!=0 )
    {
        /* Cannot compile regular expression */
        printf("Cannot compile regular expression");
        return NULL;
    }
    
    while (stop == 0)
    {
        pthread_mutex_lock(&mutex_RS);
        if (pthData->plSearchQueue->iNodes > 0)
        {
            search = 1;
            pnTemp=get_node_top(pthData->plSearchQueue);
            file_end = pthData->iFile_end;
            
        }
        pthread_mutex_unlock(&mutex_RS);
        if (search>0)
        {
            if (SearchForMaskInString(pnTemp->szLine,&preg) == 0)
            {
                pthread_mutex_lock(&mutex_SW);
                
                list_tail_add(pthData->plWrightQueue, pnTemp);
                
                pnTemp = NULL;
                
                pthread_mutex_unlock(&mutex_SW);
            }
            else
            {
                destroy_node(pnTemp);
                pnTemp=NULL;
            }
        }
        if ((search == 0) && (file_end == 1))
        {
            stop = 1;
        }
    }
    
    regfree(&preg);
    return NULL;
}
/* Function for writing thread */
void* writer_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    int stop = 0, write = 0, counter = 0, file_end = 0;
    node_t *pnTemp = NULL;
    
    printf("Lines coresponding to mask '%s':",szMask);
    
    while ((stop == 0) || (counter != iMaxLines))
    {
        pthread_mutex_lock(&mutex_SW);
        if (pthData->plWrightQueue->iNodes > 0) {
            write = 1;
            pnTemp=get_node_top(pthData->plWrightQueue);
            file_end = pthData->iFile_end;
        }
        pthread_mutex_unlock(&mutex_SW);
        if (write>0) {
            printf("%s%s",pnTemp->szLine,szSeparator);
            counter++;
            destroy_node(pnTemp);
            pnTemp = NULL;
        }
        if (counter==iMaxLines)
        {
            pthread_mutex_lock(&mutex_CP);
            pthData->iMax_lines = 1;
            pthread_mutex_unlock(&mutex_CP);
            stop = 1;
        }
        if ((write == 0) && (file_end == 1)) {
            stop = 1;
        }
    }
    return NULL;
}


int main(int argc, const char * argv[])
{
    /* input parametters checking */
    if ((argc < 3) || (argc > 6))
    {
        /* Print error: not enaugh input parameters */
        printf("Incoorect input parameters \n");
        return ERROR;
    }
    
    /* Default parammeters set */
    szFilePath = argv[1]; szMask = argv[2];
    if (argv[3]) { iMaxLines = atoi(argv[3]); } else { iMaxLines    = 10000; }
    if (argv[4]) { iScanTail = atoi(argv[4]); } else { iScanTail    = 0;     }
    if (argv[5]) { szSeparator = argv[5];     } else { szSeparator  = "\n";  }
    
    /* Common data structure for threads */
    comon_data_t stThreadData;
    stThreadData.iFile_end = 0; stThreadData.iMax_lines = 0; stThreadData.search = 0; stThreadData.write =0 ;
    if ( ((stThreadData.plSearchQueue = list_init())==NULL) || ((stThreadData.plWrightQueue = list_init())==NULL) )
    {
        return ERROR;
    }
    
    /* List's for queues */
    list_t *plQueue_RS, *plQueue_SW;
    if ( ((plQueue_RS = list_init())==NULL) || ((plQueue_SW = list_init())==NULL) )
    {
        return ERROR;
    }
    
    /* Pointers to threads */
    pthread_t *pthReader = NULL, *pthSearcher = NULL, *pthWriter = NULL;
    
    /* Trying to create threads */
    if (pthread_create(pthReader, NULL, &reader_thread, (void* )&stThreadData) != 0)
    {
        printf("Cannot creat thread for file reader");
        return ERROR;
    }
    if (pthread_create(pthSearcher, NULL, &search_thread, (void*)&stThreadData) != 0)
    {
        printf("Cannot creat thread for search");
        return ERROR;
    }
    if (pthread_create(pthWriter, NULL, &writer_thread, (void*)&stThreadData) != 0)
    {
        printf("Cannot creat thread for write");
        return ERROR;
    }
    /* wait until all threads are stopped */
    pthread_join(*pthReader, NULL);
    pthread_join(*pthWriter, NULL);
    pthread_join(*pthSearcher, NULL);
    
    /* Destroying everything */
    list_destroy(stThreadData.plWrightQueue);
    list_destroy(stThreadData.plSearchQueue);
    return SUCCESS;
}
