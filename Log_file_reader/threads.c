#include "threads.h"
#include "list_t.h"

/** Global mutexes:
 @mutex_RS - read->search mutex;
 @mutex_SW - search->write mutex;
 @mutex_CP - common parameters mutex.
 */
extern pthread_mutex_t mutex_RS, mutex_SW, mutex_CP;

/* Global semaphores:
 @semaphor_RS - read->search semaphores;
 @semaphor_SW - search->write semaphores;
 @semaphor_CP - common parameters semaphores.
 */
extern sem_t *semaphor_RS, *semaphor_SW, *semaphor_CP;


/* Init common data structure */
comon_data_t* threads_data_init(input_t *pInputData)
{
    comon_data_t *pcdTemp = malloc(sizeof(comon_data_t));
    pcdTemp->plSearchQueue = list_init();
    pcdTemp->plWrightQueue = list_init();
    pcdTemp->uiFlags       = 0;
    pcdTemp->pInputData    = pInputData;
    return pcdTemp;
}

void counter_check(int *iCounter)
{
    if (*iCounter>20000)
        usleep(1000);
}

int push_new_node_to_queue(node_t *pnTemp, list_t *plQueue, pthread_mutex_t *mutex, sem_t *semaphore)
{
    int iCount=0;
    /* Lock the mutex */
    pthread_mutex_lock(mutex);
    /* Insert the new node in the end of the queue */
    list_tail_add(plQueue, pnTemp);
    /* Get total amount of nodes in queue */
    iCount = plQueue->iNodes;
    /* Unlock the mutex */
    pthread_mutex_unlock(mutex);
    /* Increase the semaphor */
    sem_post(semaphore);
    /*Return totall amount of nodes*/
    return iCount;
}

node_t * get_node_from_queue( list_t *plQueue, pthread_mutex_t *mutex, int* flag)
{
    pthread_mutex_lock(mutex);
    if (plQueue->iNodes>0)
    {
        node_t* pnTemp = get_node_top(plQueue);
        pthread_mutex_unlock(mutex);
        *flag=1;
        return pnTemp;
        
    }
    else
    {
        pthread_mutex_unlock(mutex);
        *flag=0;
        return NULL;
    }
}

int thread_flag_check(int *uiFlag, int thread_id)
{
    if (sem_trywait(semaphor_CP)==0)
    {
        pthread_mutex_lock(&mutex_CP);
        if ((*uiFlag)&1<<3)
        {
            pthread_mutex_unlock(&mutex_CP);
            sem_post(semaphor_CP);
            return 1;
        }
        else
        {
            switch (thread_id)
            {
                case READER:
                    if ( (*uiFlag)&1<<0 )
                    {
                        (*uiFlag)|=1<<2;
                        pthread_mutex_unlock(&mutex_CP);
                        sem_post(semaphor_CP);
                        return 1;
                    }
                    else break;
                case SEARCHER:
                    if ( (*uiFlag)&1<<0 || (*uiFlag)&1<<2)
                    {
                        (*uiFlag)|=1<<1;
                        pthread_mutex_unlock(&mutex_CP);
                        sem_post(semaphor_CP);
                        return 1;
                    }
                    else break;
                case WRITER:
                    if ( (*uiFlag)&1<<1)
                    {
                        (*uiFlag)|=1<<0;
                        pthread_mutex_unlock(&mutex_CP);
                        sem_post(semaphor_CP);
                        return 1;
                    }
                    else break;
                default:
                    pthread_mutex_unlock(&mutex_CP);
                    return ERROR;
            }
        }
    }
    pthread_mutex_unlock(&mutex_CP);
    sem_post(semaphor_CP);
    return 0;
}

void thread_flag_post(int *flag, int iValue)
{
    pthread_mutex_lock(&mutex_CP);
    *flag += iValue;
    pthread_mutex_unlock(&mutex_CP);
    sem_post(semaphor_CP);
}

int write_output_in_file(char *szLine, int *iFlag, int *iAmount, char* szOptLine, FILE *optFile)
{
    switch (*iFlag%10)
    {
        case W_START:
            if (szOptLine!=NULL)
            {
                optFile = fopen("Result.txt", "w+");
                fprintf(optFile,"Lines coresponding to mask %s: \n",szOptLine);
                printf("Lines coresponding to mask %s in Resulst.txt \n",szOptLine);
                
                (*iFlag)++;
                break;
            }
            else return ERROR;
        case W_FIRST:
            if (szLine!=NULL && optFile!=NULL)
            {
                fprintf(optFile,"%s",szLine);
                (*iFlag)++;
                (*iAmount)++;
                break;
            }
            else return ERROR;
        case W_CURENT:
            if (szLine!=NULL && szOptLine!=NULL && optFile!=NULL)
            {
                fprintf(optFile,"%s%s",szOptLine,szLine);
                (*iAmount)++;
                break;
            }
            else return ERROR;
        case W_LAST:
            if (iAmount!=NULL && szOptLine!=NULL)
            {
                printf("Log file Readed, Corresponding lines in Result.txt\n");
                if ((*iAmount)==0)
                {
                    fprintf(optFile,"There is no lines corresponding to '%s' \n",szOptLine);
                    printf("There is no lines corresponding to '%s' \n",szOptLine);
                    fclose(optFile);
                    break;
                }
                else
                {
                    fprintf(optFile,"Totall amount of lines coresponding to '%s': %d\n", szOptLine, *iAmount);
                    printf("Totall amount of lines coresponding to '%s': %d\n", szOptLine, *iAmount);
                    fclose(optFile);
                    break;
                }
            } else return ERROR;
        default:
            return ERROR;
    }
    
    
    return SUCCESS;
}
int show_output_on_console(char *szLine, int *iFlag, int *iAmount, char* szOptLine)
{
    switch (*iFlag%10)
    {
        case W_START:
            if (szOptLine!=NULL)
            {
                printf("Lines coresponding to mask %s: \n",szOptLine);
                (*iFlag)++;
                break;
            }
            else return ERROR;
        case W_FIRST:
            if (szLine!=NULL)
            {
                printf("%s",szLine);
                (*iFlag)++;
                (*iAmount)++;
                break;
            }
            else return ERROR;
        case W_CURENT:
            if (szLine!=NULL && szOptLine!=NULL)
            {
                printf("%s%s",szOptLine,szLine);
                (*iAmount)++;
                break;
            }
            else return ERROR;
        case W_LAST:
            if (iAmount!=NULL && szOptLine!=NULL)
            {
                if ((*iAmount)==0)
                {
                    printf("Log file Readed. \n");
                    printf("There is no lines corresponding to '%s' \n",szOptLine);
                    break;
                }
                else
                {
                    printf("\nLog file Readed. \n");
                    printf("Totall amount of lines coresponding to '%s': %d\n",
                           szOptLine, *iAmount);
                    break;
                }
            } else return ERROR;
            
        default:
            return ERROR;
    }
    
    return SUCCESS;
}

int write_output_line(char *szLine, int *iFlag, int *iAmount, char* szOptLine, FILE *optFile)
{
    if (*iFlag<10)
    {
        return show_output_on_console(szLine,iFlag,iAmount,szOptLine);
    }
    else
    {
        return write_output_line(szLine, iFlag, iAmount, szOptLine, optFile);
    }
}

/* Tread Functions: */
/* Function for file reader thread */
void* reader_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    node_t *pnTemp = NULL;
    int i = 0, j = 0, iCounter = 0, iCurent_LineSize = 0;
    FILE *file = NULL;
    long lFSize = 0, lBytesToReadOnce = 0, lI = 0;
    char *szLine = NULL, *Block = NULL;
    
    /* try to open file for reading */
    if ( (file = fopen((const char*)pthData->pInputData->szFilePath, "r")) == NULL )
    {
        /* File cannot be oppened */
        printf("Can't open the file \n");
        /* We are not alone, so lets inform everyone about it*/
        thread_flag_post(&pthData->uiFlags, TH_ERROR);
        /* Posting searcher semaphore becusi it still wait to do something*/
        sem_post(semaphor_RS);
        return NULL;
    }
    /* if file opened  */
    else
    {
        /* Allocate memory for lines */
        szLine = malloc(BIGEST_LINE+2);
        iCurent_LineSize = BIGEST_LINE;
        
        /* Getting file size */
        fseek(file, 0, SEEK_END);
        lFSize = ftell(file);
        
        /* We determine how many bytes we read at once */
        if (lFSize>BLOCK_SIZE)
        {
            Block = malloc(BLOCK_SIZE +2);
            lBytesToReadOnce = BLOCK_SIZE;
        }
        else
        {
            lBytesToReadOnce = lFSize;
            Block = malloc(lFSize + 2 );
        }
        
        /* Where do we need to start */
        if (pthData->pInputData->iScanTail == 0)
        {/*   Reading file from the beginig   */
            
            fseek(file, 0, SEEK_SET);
            /*Position in the readed line*/
            j = 0;
            
            /* Let's read! */
            while (lFSize>0)
            {
                /* Lets check that we still need to read */
                if (thread_flag_check(&pthData->uiFlags, READER)!=0)
                {
                    break;
                }                /* and now we will read and scan another block */
                fread(Block, sizeof(char), lBytesToReadOnce, file);
                /* Decreasing the amount of unread bytes */
                lFSize -= lBytesToReadOnce;
                /* Scaning block for lines */
                for (i = 0; i<lBytesToReadOnce; i++)
                {
                    if (j == (iCurent_LineSize-1) ){ szLine = realoc_string(szLine, &iCurent_LineSize); }
                    if ( ((szLine[j] = Block[i]) =='\n') || j==(MAX_LINE - 2) || Block[i]=='\0')
                    {
                        szLine[j]='\0';
                        /* Creating a new node for this line */
                        pnTemp = node_init(szLine);
                        /* Transfer a new node to the search queue */
                        iCounter = push_new_node_to_queue(pnTemp, pthData->plSearchQueue,
                                                         &mutex_RS, semaphor_RS);
                        j = 0; /* position in line to 0 */
                        /* Give a break to the search thread, if there is too many nodes in queue */
                        counter_check(&iCounter);
                    }
                    else{j++;} /* increment line position */
                }
                /* if amount of unread bytes is smaller than a read block */
                lBytesToReadOnce = max(&lBytesToReadOnce,&lFSize);
            }
        }
        else
        {
            /* Scaning from the end of file */
            j = 0; long iPosition = 0;
            /* Position in file where do we start to read */
            while (1)
            {
                /* Lets check that we still need to read */
                if (thread_flag_check(&pthData->uiFlags, READER)!=0)
                {
                    break;
                }
                
                /* and now we will read and scan */
                iPosition += lBytesToReadOnce;
                fseek(file, -iPosition, SEEK_END);
                fread(Block, sizeof(char), lBytesToReadOnce, file);
                
                /* Scaning block for lines */
                for (lI = lBytesToReadOnce-1; lI>=0; lI--)
                {
                    if (j == (iCurent_LineSize-1))
                    {
                        szLine = realoc_string(szLine, &iCurent_LineSize);
                    }
                    if ( ((szLine[j] = Block[lI]) =='\n') || j==(MAX_LINE - 2) || Block[lI] =='\0')
                    {
                        array_swap(szLine, &j);
                        szLine[j]=0;
                        /* Creating a new node for this line */
                        pnTemp = node_init(szLine);
                        /* Transfer a new node to the search queue */
                        iCounter = push_new_node_to_queue(pnTemp, pthData->plSearchQueue,
                                                          &mutex_RS, semaphor_RS);
                        j = 0; /* position in line to 0 */
                        /* Give a break to the search thread, if there is too many nodes in queue */
                        counter_check(&iCounter);
                    }
                    else{j++;} /* increment line position */
                }
                lFSize -= lBytesToReadOnce;
                if (lFSize==0)
                {
                    /* Sending the last line */
                    
                    szLine[j+1] = '\0';
                    array_swap(szLine, &j);
                    pnTemp = node_init(szLine);
                    
                    /* Transfer a new node to the search queue */
                    iCounter = push_new_node_to_queue(pnTemp, pthData->plSearchQueue,
                                                      &mutex_RS, semaphor_RS);
                    break;
                }
                /* if remeaning file size is smaller tha read at once blokk */
                lBytesToReadOnce = max(&lBytesToReadOnce,&lFSize);
            }
        }
        /* Give a signal to other thread that file is readed */
        thread_flag_post(&pthData->uiFlags,TH_FILE_END);
        /* Increase the read->search semaphor*/
        sem_post(semaphor_RS);
        /* Close the file and free allocated memory*/
        fclose(file); free(Block); free(szLine);
    }
    return NULL;
}

/* Function for searching thread */
void* search_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    node_t *pnTemp = NULL;
    search_t *psSearch = NULL;
    int iSearch = 0;
    char *szMask_temp;
    
    szMask_temp=malloc(strlen(pthData->pInputData->szMask)+2);
    strlcpy_udev(szMask_temp, pthData->pInputData->szMask, strlen(pthData->pInputData->szMask)+1);
    
    /* Creating search_t structure*/
    if ((psSearch = compile_search_expression(szMask_temp)) == NULL)
    {
        /* Can't create the Search_t structure */
        printf("Can't create the Search_t structure \n");
        /* We are not alone, so lets inform everyone about it*/
        thread_flag_post(&pthData->uiFlags, TH_ERROR);
        /* Increase writer semaphore */
        sem_post(semaphor_SW);
        return NULL;
    }
    while (1)
    {
        /* Lets wait untill we have some thing to scan */
        sem_wait(semaphor_RS);
        /* If we have the line for search */
        pnTemp = get_node_from_queue(pthData->plSearchQueue, &mutex_RS, &iSearch);
        if (iSearch == 1)
        {
            /* let's search in the line */
            if (  psSearch->search(pnTemp->szLine, (void *)psSearch) == FOUND)
            {
                /* if we found something */
                push_new_node_to_queue(pnTemp, pthData->plWrightQueue, &mutex_SW, semaphor_SW);
                iSearch=0;
            }
            else
            {
                /* nothing we need is in this line, let's destroy it */
                destroy_node(pnTemp); pnTemp=NULL;
                iSearch=0;
            }
        }
        else if (iSearch == 0)
        {
            /* if we have nothing to do, lets check that we still need to work */
            
            if (thread_flag_check(&pthData->uiFlags, SEARCHER)!=0)
            {
                break;
            }
        }
    }
    /* Increase the writer semaphore */
    sem_post(semaphor_SW);
    search_destroy(psSearch);
    free(szMask_temp);
    return NULL;
}


/* Function for writer thread */
void* writer_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    int write = 0, max_lines = 0, iFlag = 0 + (pthData->pInputData->iOut * 10);
    int iAmount =0;
    node_t *pnTemp = NULL;
    FILE *file = NULL;
    char *szMask= pthData->pInputData->szMask, *szSeparator=pthData->pInputData->szSeparator;
    
    /*Write the title to the results in console or in file (depends on flag)*/
    if (write_output_line(NULL, &iFlag, NULL, szMask, file)==ERROR)
    {
        thread_flag_post(&pthData->uiFlags, TH_ERROR);
        return NULL;
    }
    max_lines = pthData->pInputData->iMaxLines;
    while (1)
    {
        /* Wait on writer semaphore untill we have something to do */
        sem_wait(semaphor_SW);
        
        pnTemp = get_node_from_queue(pthData->plWrightQueue, &mutex_SW,&write);
        /* If we have something to write */
        
        if ( write ==1)
        {
            if (write_output_line(pnTemp->szLine, &iFlag, &iAmount, szSeparator, file)==ERROR)
            {
                thread_flag_post(&pthData->uiFlags, TH_ERROR);
                return NULL;
            }
            max_lines--; write=0;
            destroy_node(pnTemp); pnTemp = NULL;
            
            /* Are we have every line we need? */
            if (max_lines == 0) {break;}
        }
        /* Is there anyone else still working */
        else if (write == 0)
        {
            /* We need to check, if something stil in search or read */
            if (thread_flag_check(&pthData->uiFlags, WRITER)!=0) { break; }
        }
    }
    iFlag = W_LAST + (pthData->pInputData->iOut * 10);
    /* Printing some end information */
    if (write_output_line(NULL, &iFlag, &iAmount, szMask, file)==ERROR)
    {
        thread_flag_post(&pthData->uiFlags, TH_ERROR);
        return NULL;
    }
    /* We done here, Everyone needs to know about it */
    thread_flag_post(&pthData->uiFlags,TH_MAX_LINES);
    
    return NULL;
}
