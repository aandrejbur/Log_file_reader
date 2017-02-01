/* Include system librarys */
#include <pthread.h>

/* Include own librarys */
#include "list_t.h"
#include "usfull_utilities.h"

/* ./execute -f-file_path -m-mask -l-max_lines -t-scan_tail -s-separator */

/* Block size to read = 4MB */
#define BLOCK_SIZE 4194304
#define BIGEST_LINE 2048

/* Codes to end program */
#define FILE_END    10
#define MAX_LINES   20

/* The answer to main question */
#define STOP        42

/* Global mutexes: 1 - read->search mutex, 2 - search->write mutex, 3 - common parameters mutex  */
pthread_mutex_t mutex_RS, mutex_SW, mutex_CP;

/* Input parameters */
const char *szMask, *szFilePath, *szSeparator;
int iScanTail, iMaxLines, iAmount;


/* Struct for threads */
typedef struct comon_data
{
    
    /* Queues for search and write */
    list_t *plSearchQueue, *plWrightQueue;
    /* Flags for stoping the process */
    int iFile_end, iMax_lines, Search_is_done, iError;
    
    
    
} comon_data_t;


/* Search for mask in string, Return 0 if finded*/
int SearchForMaskInString(char * szString, regex_t *preg)
{
    return regexec(preg, szString, 0, NULL, 0);
}


/* Tread Functions: */
/* Function for file reader thread */
void* reader_thread(void *pThreadData)
{
    
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    node_t *pnTemp = NULL;
    int i = 0, j = 0, direction = 0;;
    FILE *file;
    long lFSize = 0, lBytesToReadOnce = 0;
    char szLine[BIGEST_LINE], *Block = NULL;
    
    /* try to open file for reading */
    if ( (file = fopen(szFilePath, "r")) == NULL )
    {
        /* File cannot be oppened */
        printf("File cannot be oppened");
        
        /* We are not alone, so lets inform everyone about it*/
        pthread_mutex_lock(&mutex_CP);
        pthData->iError = 1;
        pthread_mutex_unlock(&mutex_CP);
        return NULL;
    }
    /* if file opened  */
    else
    {
        direction = iScanTail;
        /* Getting file size */
        fseek(file, 0, SEEK_END);
        lFSize = ftell(file);
        /* We determine how many bytes we read at a time */
        if (lFSize>BLOCK_SIZE)
        {
            Block = malloc(BLOCK_SIZE);
            lBytesToReadOnce = BLOCK_SIZE;
        }
        else
        {
            lBytesToReadOnce = lFSize;
            Block = malloc(lFSize);
        }
        /* Where do we need to start */
        if (direction == 0)
        {
            /*   Reading file from the beginig   */
            rewind(file);
            /*Position in the readed line*/
            j = 0;
            /* Let's read! */
            while (lFSize>0)
            {
                /* Lets check that we still need to read */
                pthread_mutex_lock(&mutex_CP);
                if (pthData->iMax_lines != 0)
                {
                    pthData->iFile_end = 1;
                    pthread_mutex_unlock(&mutex_CP);
                    break;
                }
                else
                {
                    pthread_mutex_unlock(&mutex_CP);
                }
                /* and now we will read and scan */
                fread(Block, sizeof(char), lBytesToReadOnce, file);
                lFSize -=lBytesToReadOnce;
                /* Scaning block for lines */
                for (i = 0; i<lBytesToReadOnce; i++)
                {
                    if ( (szLine[j] = Block[i]) =='\n')
                    {
                        szLine[j]='\0';
                        /* Creating a new node for this line */
                        pnTemp = node_init(szLine);
                        /* Give a signal to other thread for searching */
                        pthread_mutex_lock(&mutex_RS);
                        /* Insert the new node in the end of the search queue */
                        list_tail_add(pthData->plSearchQueue, pnTemp);
                        /* Unlock the mutex */
                        pthread_mutex_unlock(&mutex_RS);
                        j = 0;
                    }
                    j++;
                }
                if (lFSize<=lBytesToReadOnce)
                {
                    lBytesToReadOnce = lFSize;
                }
            }
        }
        else
        {
            long lI = 0;
            /* Scaning from the end of file */
            j = 0;
            while (lFSize>0)
            {
                /* Lets check that we still need to read */
                pthread_mutex_lock(&mutex_CP);
                if ((pthData->iMax_lines != 0) || (pthData->iError !=0))
                {
                    pthData->iFile_end = 1;
                    pthread_mutex_unlock(&mutex_CP);
                    break;
                }
                else
                {
                    pthread_mutex_unlock(&mutex_CP);
                }
                /* and now we will read and scan */
                fseek(file, -lBytesToReadOnce+1, SEEK_CUR);
                fread(Block, sizeof(char), lBytesToReadOnce, file);
                lFSize -=lBytesToReadOnce;
                
                
                /* Scaning block for lines */
                for (lI = lBytesToReadOnce; lI>0; lI--)
                {
                    if ( (szLine[j] = Block[lI]) =='\n')
                    {
                        szLine[j]='\0';
                        array_swap(szLine, j);
                        /* Creating a new node for this line */
                        pnTemp = node_init(szLine);
                        /* Give a signal to other thread for searching */
                        pthread_mutex_lock(&mutex_RS);
                        /* Insert the new node in the end of the search queue */
                        list_tail_add(pthData->plSearchQueue, pnTemp);
                        /* Unlock the mutex */
                        pthread_mutex_unlock(&mutex_RS);
                        j = 0;
                    }
                    j++;
                }
                if (lFSize<=lBytesToReadOnce)
                {
                    lBytesToReadOnce = lFSize;
                }

            }
            
        }
        /* Give a signal to other thread that file is readed */
        pthread_mutex_lock(&mutex_CP);
        pthData->iFile_end = 1;
        pthread_mutex_unlock(&mutex_CP);
        /* Close the file */
        fclose(file);
        /* Free the memory */
        free(Block);
    }
    return NULL;
}
/* Function for searching thread */
void* search_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    node_t *pnTemp = NULL;
    regex_t preg;
    int search = 0, counter = 0;
    
    /* Compiling regular expression */
    if ((regcomp(&preg, szMask, 0))!=0 )
    {
        /* Cannot compile regular expression */
        printf("Cannot compile regular expression");
        pthread_mutex_lock(&mutex_CP);
        pthData->iError = 1;
        pthread_mutex_unlock(&mutex_CP);
        
        return NULL;
    }
    
    while (1)
    {
        /* Try to get the line from the search queue */
        pthread_mutex_lock(&mutex_RS);
        if (pthData->plSearchQueue->iNodes > 0)
        {
            search++;
            pnTemp=get_node_top(pthData->plSearchQueue);
        }
        pthread_mutex_unlock(&mutex_RS);
        /* If we have the line for search */
        if (search>0)
        {
            /* let's search in the line */
            if (SearchForMaskInString(pnTemp->szLine,&preg) == 0)
            {
                /* if we found something */
                pthread_mutex_lock(&mutex_SW);
                /* send the line to writer queue */
                list_tail_add(pthData->plWrightQueue, pnTemp);
                pnTemp = NULL;
                pthread_mutex_unlock(&mutex_SW);
                search--;
            }
            else
            {
                /* nothing we need is in this line, let's destroy it */
                destroy_node(pnTemp);
                pnTemp=NULL;
                search--;
            }
        }
        /* if we have nothing to do, lets check that we still need to work */
        else if (search == 0)
        {
            if (counter!=10)
            {
                counter++;
            }
            else
            {
                /* a check if the file have ended or we have all needed lines */
                pthread_mutex_lock(&mutex_CP);
                if ( (pthData->iFile_end!=0) || (pthData->iMax_lines!=0) || (pthData->iError!=0))
                {
                    pthData->Search_is_done = 1;
                    pthread_mutex_unlock(&mutex_CP);
                    break;
                }else
                {
                    pthread_mutex_unlock(&mutex_CP);
                    counter = 0;
                }
            }
            
        }
    }
    
    regfree(&preg);
    return NULL;
}
/* Function for writing thread */
void* writer_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    int counter = 0, write = 0, max_lines = 0, first = 0;
    node_t *pnTemp = NULL;
    
    printf("Lines coresponding to mask '%s': \n",szMask);
    
    max_lines = iMaxLines;
    while (1)
    {
        /* lets try to get a line for aur work */
        pthread_mutex_lock(&mutex_SW);
        if (pthData->plWrightQueue->iNodes > 0) {
            write++;
            pnTemp=get_node_top(pthData->plWrightQueue);
            
        }
        pthread_mutex_unlock(&mutex_SW);
        /* If we have something to write */
        if (write>0)
        {
            if (first == 0) {
                printf("%s",pnTemp->szLine);
                first++;
            }else
            {
                printf("%s%s",szSeparator,pnTemp->szLine);
            }
            max_lines--;
            write--;
            destroy_node(pnTemp);
            pnTemp = NULL;
        }
        /* Are we have every line we need? */
        if (max_lines == 0)
        {
            /* Everyone need\s to know about it */
            pthread_mutex_lock(&mutex_CP);
            pthData->iMax_lines = 1;
            pthread_mutex_unlock(&mutex_CP);
            break;
        }
        /* Is there anyone else still working */
        if (write == 0)
        {
            /* Lets give some time to other threads to work */
            if (counter!=10)
            {
                counter++;
            }
            else
            {
                /* We need to check, if something stil in search or read */
                pthread_mutex_lock(&mutex_CP);
                if ( (pthData->iError!=0) || ( (pthData->iFile_end!=0)&&(pthData->Search_is_done!=0)) )
                {
                    pthData->iMax_lines = 1;
                    pthread_mutex_unlock(&mutex_CP);
                    break;
                }
                else
                {
                    pthread_mutex_unlock(&mutex_CP);
                    counter=0;
                }
            }
        }
    }
    iAmount = iMaxLines-max_lines;
    return NULL;
}


int main(int argc, const char * argv[])
{
    /* input parametters checking */
    if ((argc < 3) || (argc > 6))
    {
        /* Print error: not enaugh input parameters */
        printf("Incoorect input parameters \n");
        //szFilePath = "log.txt"; szMask = "Net";
        return ERROR;
    }
    else {
        /* Default parammeters set */
        szFilePath = argv[1]; szMask = argv[2];
    }
    if (argv[3])
    {
        iMaxLines = atoi(argv[3]);
    }
    else
    {
        iMaxLines    = 10;
    }
    if (argv[4])
    {
        iScanTail = atoi(argv[4]);
    }
    else
    {
        iScanTail    = 0;
    }
    if (argv[5])
    {
        szSeparator = argv[5];
    }
    else
    {
        szSeparator  = "\n";
    }
    
    /* Initialisation of mutexes */
    pthread_mutex_init(&mutex_SW, NULL);
    pthread_mutex_init(&mutex_RS, NULL);
    pthread_mutex_init(&mutex_CP,NULL);
    
    
    /* Common data structure for threads */
    comon_data_t stThreadData;
    stThreadData.iFile_end = 0;
    stThreadData.iMax_lines = 0;
    stThreadData.Search_is_done = 0;
    stThreadData.iError = 0;
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
    pthread_t pthReader, pthSearcher, pthWriter;
    
    /* Trying to create threads */
    if (pthread_create(&pthReader, NULL, &reader_thread, (void* )&stThreadData) != 0)
    {
        printf("Cannot creat thread for file reader");
        return ERROR;
    }
    
    if (pthread_create(&pthSearcher, NULL, &search_thread, (void*)&stThreadData) != 0)
    {
        printf("Cannot creat thread for search");
        return ERROR;
    }
    if (pthread_create(&pthWriter, NULL, &writer_thread, (void*)&stThreadData) != 0)
    {
        printf("Cannot creat thread for write");
        return ERROR;
    }
    /* wait until all threads are stopped */
    pthread_join(pthReader, NULL);
    pthread_join(pthWriter, NULL);
    pthread_join(pthSearcher, NULL);
    
    /* Destroying mutexes */
    pthread_mutex_destroy(&mutex_RS);
    pthread_mutex_destroy(&mutex_SW);
    pthread_mutex_destroy(&mutex_CP);
    
    /* Destroying everything */
    list_destroy(stThreadData.plWrightQueue);
    list_destroy(stThreadData.plSearchQueue);
    
    /* Print that we succsesfully done */
    printf("\nLog file Readed succesfully, totall amount of lines coresponding to mask: %d\n", iAmount);
    return SUCCESS;
}
