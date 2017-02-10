#include "threads.h"

/**
 * Global mutexes:
 * @mutex_RS - read->search mutex;
 * @mutex_SW - search->write mutex;
 * @mutex_CP - common parameters mutex.
 */
pthread_mutex_t mutex_RS, mutex_SW, mutex_CP;

/**
 * Global semaphores pointers:
 * @semaphor_RS - read->search semaphores;
 * @semaphor_SW - search->write semaphores;
 * @semaphor_CP - common parameters semaphores.
 */
sem_t *semaphor_RS, *semaphor_SW, *semaphor_CP;

/**
 * Global pointers to posix threads
 * @pthReader   - reader thread
 * @pthSearcher - searcher thread
 * @pthWriter   - writer thread
 */
pthread_t pthReader, pthSearcher, pthWriter;

/* Init threads common data structure */
common_data_t* threads_data_init(struct input_t *pInputData)
{
    common_data_t *pcdTemp = malloc(sizeof(common_data_t));
    pcdTemp->plSearchQueue = list_init();
    pcdTemp->plWrightQueue = list_init();
    pcdTemp->uiFlags       = 0;
    pcdTemp->pInputData    = pInputData;
    return pcdTemp;
}

/* Try to init mutexes and semaphores */
int threads_sync_init()
{
    /* Initialisation of mutexes and semaphores*/
    
    if ( (pthread_mutex_init(&mutex_RS, NULL)!=0)  ||
        ( (semaphor_RS = sem_open("semafor_RS", O_CREAT, 0644, 0)) == SEM_FAILED) )
    {
        printf("Cannot init thread synchronization for Reader Thread");
        return ERROR;
    }
    if ( (pthread_mutex_init(&mutex_SW, NULL)!=0)  ||
        ( (semaphor_SW = sem_open("semafor_SW", O_CREAT, 0644, 0))==SEM_FAILED) )
    {
        printf("Cannot init thread synchronization for Searcher Thread");
        return ERROR;
    }
    if ( (pthread_mutex_init(&mutex_CP, NULL)!=0)  ||
        ( (semaphor_CP = sem_open("semafor_CP", O_CREAT, 0644, 0)) ==SEM_FAILED) )
    {
        printf("Cannot init thread synchronization for Writer Thread");
        return ERROR;
    }
    return SUCCESS;
}

/* Try to start threads */
int threads_start(common_data_t *pstThreadData)
{
    /* Trying to create threads */
    if (pthread_create(&pthReader, NULL, &reader_thread, (void*)pstThreadData) != 0)
    {
        printf("Cannot create thread for file reader");
        return ERROR;
    }
    if (pthread_create(&pthSearcher, NULL, &search_thread, (void*)pstThreadData) != 0)
    {
        printf("Cannot create thread for search");
        return ERROR;
    }
    if (pthread_create(&pthWriter, NULL, &writer_thread, (void*)pstThreadData) != 0)
    {
        printf("Cannot create thread for write");
        return ERROR;
    }
    return SUCCESS;
}

/* Wait end of threads */
void threads_join()
{
    pthread_join(pthReader,     NULL);
    pthread_join(pthSearcher,   NULL);
    pthread_join(pthWriter,     NULL);
}

/* destroy semaphores and mutexes */
void threads_sync_destroy()
{
    pthread_mutex_destroy(&mutex_RS); sem_close(semaphor_RS); sem_unlink("semafor_RS");
    pthread_mutex_destroy(&mutex_SW); sem_close(semaphor_SW); sem_unlink("semafor_SW");
    pthread_mutex_destroy(&mutex_CP); sem_close(semaphor_CP); sem_unlink("semafor_CP");
}


/* Post the flag in common data */
void thread_flag_post(int *flag, int iValue)
{
    pthread_mutex_lock(&mutex_CP);
    *flag += iValue;
    pthread_mutex_unlock(&mutex_CP);
    sem_post(semaphor_CP);
}

/* Check flags */
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


/* Insert new node in tail of selected queue */
int push_new_node_to_queue(node_t *pnTemp, list_t *plQueue,
                           pthread_mutex_t *mutex, sem_t *semaphore)
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

/* Get node from head of the selected queue */
node_t* get_node_from_queue(list_t *plQueue, pthread_mutex_t *mutex, int* flag)
{
    /* Lock the mutex */
    pthread_mutex_lock(mutex);
    /* Check if the queue is empty */
    if (plQueue->iNodes>0)
    {
        /* get the node from queue */
        node_t* pnTemp = get_node_top(plQueue);
        /* ulock the mutex */
        pthread_mutex_unlock(mutex);
        /* Set signal flag in 1 */
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

/* Reader thread function*/
void* reader_thread(void *pThreadData)
{
    common_data_t *pthData = (common_data_t*)pThreadData;
    reader_t *pReader_t = NULL;
    
    /* try to open file for reading */
    if ( ( pReader_t = reader_t_init(pthData->pInputData->szFilePath)) == NULL )
    {
        /* File cannot be oppened */
        printf("Can't open the file \n");
        
        /* We are not alone, so lets inform everyone about it*/
        thread_flag_post(&pthData->uiFlags, TH_ERROR);
        
        /* Posting searcher semaphore becusi it still wait to do something*/
        sem_post(semaphor_RS);
        return NULL;
    }
    /* Where do we need to start */
    if (pthData->pInputData->iScanTail == 0)
    {
        /*   Reading file from the beginig   */
        if (read_file_head(pthData, pReader_t) == ERROR)
        {
            thread_flag_post(&pthData->uiFlags, TH_ERROR);
            return NULL;
        }
    }
    else
    {
        if (read_file_tail(pthData, pReader_t) == ERROR)
        {
            thread_flag_post(&pthData->uiFlags, TH_ERROR);
            return NULL;
        }
    }
    /* Give a signal to other thread that file is readed */
    thread_flag_post(&pthData->uiFlags,TH_FILE_END);
    
    /* Increase the read->search semaphor ( unleash the searcher )*/
    sem_post(semaphor_RS);
    
    /* Destroy file context */
    reader_t_destroy(pReader_t);
    return NULL;
}

/* init file reader structure */
reader_t* reader_t_init(char* file_path)
{
    reader_t *pReader_t = malloc(sizeof(reader_t));
    if ((pReader_t->file=fopen(file_path, "r")) == NULL )
    {
        return NULL;
    }
    /* Getting file size */
    if (
        (fseek(pReader_t->file, 0, SEEK_END)<0 ) ||
        ( (pReader_t->lFileSize = ftell(pReader_t->file))<0 ) ||
        ( fseek(pReader_t->file, 0, SEEK_SET)<0 )
        )
    {
        return NULL;
    }
    
    /* We determine how many bytes we read at once */
    if (pReader_t->lFileSize>BLOCK_SIZE)
    {
        pReader_t->lBytesToReadOnce = BLOCK_SIZE;
    }
    else
    {
        pReader_t->lBytesToReadOnce = pReader_t->lFileSize;
    }
    pReader_t->Block = malloc(pReader_t->lBytesToReadOnce +2);
    
    /* Allocate memory for lines */
    if (pReader_t->lFileSize>BIGEST_LINE)
    {
        pReader_t->iCurent_LineSize = BIGEST_LINE;
    }
    else
    {
        pReader_t->iCurent_LineSize = (int)pReader_t->lFileSize;
    }
    pReader_t->szLine = malloc(pReader_t->iCurent_LineSize+2);
    
    return pReader_t;
}

/* Destroy file reader structure */
void reader_t_destroy(reader_t* pReader_t)
{
    fclose(pReader_t->file);
    
    free(pReader_t->Block);
    pReader_t->Block = NULL;
    
    free(pReader_t->szLine);
    pReader_t->szLine = NULL;
    
    free(pReader_t);
    pReader_t = NULL;
}

/* Read file from head */
int read_file_head(common_data_t *pthData, reader_t* pReader_t)
{
    if (pthData==NULL || pReader_t==NULL) { return ERROR; }
    
    int i = 0, j = 0, iCounter = 0;
    node_t *pnTemp = NULL;
    
    while (pReader_t->lFileSize>0) /* Read file block by block cycle*/
    {
        /* Lets check that we still need to read */
        if (thread_flag_check(&pthData->uiFlags, READER)!=0) { break; }
        
        /* and now we will read and scan another block */
        fread(pReader_t->Block, sizeof(char), pReader_t->lBytesToReadOnce, pReader_t->file);
        
        /* Decreasing the amount of unread bytes */
        pReader_t->lFileSize -= pReader_t->lBytesToReadOnce;
        
        /* Scaning block for lines */
        for (i = 0; i < (pReader_t->lBytesToReadOnce); i++)
        {
            if (j == (pReader_t->iCurent_LineSize) ) /* If curent possible line is too big */
            {
                /* reallocate the szline memory */
                pReader_t->szLine = realoc_string(pReader_t->szLine, &pReader_t->iCurent_LineSize);
            }
            if ( ((pReader_t->szLine[j] = pReader_t->Block[i]) =='\n') ||
                pReader_t->Block[i]=='\0'|| j==(MAX_LINE - 2))
            {
                pReader_t->szLine[j]='\0';
                /* Creating a new node for this line */
                pnTemp = node_init(pReader_t->szLine);
                
                /* Transfer a new node to the search queue */
                iCounter = push_new_node_to_queue(pnTemp, pthData->plSearchQueue,
                                                  &mutex_RS, semaphor_RS);
                j = 0; /* position in line to 0 */
                
                if (thread_flag_check(&pthData->uiFlags, READER)!=0) { break; }
                
                /* Give a break to the search thread, if there is too many nodes in queue */
                counter_check(iCounter,20000,1000);
            }
            else{ j++; } /* increment line position */
        }
        
        /* if amount of unread bytes is smaller then a read block */
        if (pReader_t->lFileSize < pReader_t->lBytesToReadOnce)
        {
            pReader_t->lBytesToReadOnce = pReader_t->lFileSize;
        }
        if ( (pReader_t->lFileSize == 0) && j!= 0)
        {
            /* Read the last line if it without '\0' or '\n' */
            pReader_t->szLine[j] = '\0';
            pnTemp = node_init(pReader_t->szLine);
            push_new_node_to_queue(pnTemp, pthData->plSearchQueue, &mutex_RS, semaphor_RS);
            break;
        }
    }
    return SUCCESS;
}

/* Read file from tail */
int read_file_tail(common_data_t *pthData, reader_t* pReader_t)
{
    if (pthData==NULL || pReader_t==NULL) { return ERROR; }
    
    int j = 0, iCounter = 0;
    node_t * pnTemp = NULL;
    long lI = 0, lPosition = 0;
    
    while (pReader_t->lFileSize>0)
    {
        /* Lets check that we still need to read */
        if (thread_flag_check(&pthData->uiFlags, READER)!=0) { return SUCCESS; }
        
        /* Position in file where do we start to read */
        lPosition += pReader_t->lBytesToReadOnce;
        fseek(pReader_t->file, -lPosition, SEEK_END);
        fread(pReader_t->Block, sizeof(char), pReader_t->lBytesToReadOnce, pReader_t->file);
        pReader_t->lFileSize -= pReader_t->lBytesToReadOnce;
        
        /* Scaning block for lines */
        for (lI = pReader_t->lBytesToReadOnce-1; lI>=0; lI--)
        {
            if (j == (pReader_t->iCurent_LineSize-1))
            {
                pReader_t->szLine = realoc_string(pReader_t->szLine, &pReader_t->iCurent_LineSize);
            }
            if ( ((pReader_t->szLine[j] = pReader_t->Block[lI]) =='\n') ||
                pReader_t->Block[lI] =='\0' || j==(MAX_LINE - 2))
            {
                array_swap(pReader_t->szLine, &j);
                pReader_t->szLine[j]='\0';
                /* Creating a new node for this line */
                pnTemp = node_init(pReader_t->szLine);
                /* Transfer a new node to the search queue */
                iCounter = push_new_node_to_queue(pnTemp, pthData->plSearchQueue,
                                                  &mutex_RS, semaphor_RS);
                j = 0; /* position in line to 0 */
                
                if (thread_flag_check(&pthData->uiFlags, READER)!=0) {
                    return SUCCESS;
                }
                
                /* Give a break to the search thread, if there is too many nodes in queue */
                counter_check(iCounter,20000,1000);
            }
            else{ j++; } /* increment line position */
        }
        if ( (pReader_t->lFileSize == 0) && j!= 0)
        {
            /* Read the last line */
            //j++;
            pReader_t->szLine[j] = '\0';
            array_swap(pReader_t->szLine, &j);
            pnTemp = node_init(pReader_t->szLine);
            push_new_node_to_queue(pnTemp, pthData->plSearchQueue, &mutex_RS, semaphor_RS);
            break;
        }
        
        /* if amount of unread bytes is smaller then a read block */
        if (pReader_t->lFileSize < pReader_t->lBytesToReadOnce)
        {
            pReader_t->lBytesToReadOnce = pReader_t->lFileSize;
        }
    }
    return SUCCESS;
}

/* Searcher thread function*/
void* search_thread(void *pThreadData)
{
    common_data_t *pthData = (common_data_t*)pThreadData;
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

/* writer thread function*/
void* writer_thread(void *pThreadData)
{
    common_data_t *pthData = (common_data_t*)pThreadData;
    writer_t *pWriter_t = NULL;
    
    int iWrite = 0, iMaxLines = pthData->pInputData->iMaxLines;
    
    if ( ( pWriter_t = writer_t_init(pthData->pInputData)) == NULL)
    {
        thread_flag_post(&pthData->uiFlags, TH_ERROR);
        printf("Cannot compute Writer context");
        return NULL;
    }
    /*Write the title to the results in console or in file (depends on flag)*/
    if (write_output_line(pWriter_t) != SUCCESS)
    {
        thread_flag_post(&pthData->uiFlags, TH_ERROR);
        return NULL;
    }
    while (1)
    {
        /* Wait on writer semaphore untill we have something to do */
        sem_wait(semaphor_SW);
        
        pWriter_t->pnTemp = get_node_from_queue(pthData->plWrightQueue, &mutex_SW,&iWrite);
        /* If we have something to write */
        if ( iWrite ==1)
        {
            if (write_output_line(pWriter_t)==ERROR)
            {
                thread_flag_post(&pthData->uiFlags, TH_ERROR);
                return NULL;
            }
            else
            {
                iMaxLines--; iWrite=0;
                destroy_node(pWriter_t->pnTemp); pWriter_t->pnTemp = NULL;
            }
            
            /* Are we have every line we need? */
            if ( iMaxLines == 0 ) { break; }
        }
        /* Is there anyone else still working */
        else if (iWrite == 0)
        {
            /* We need to check, if something stil in search or read */
            if (thread_flag_check(&pthData->uiFlags, WRITER)!=0) { break; }
        }
    }
    pWriter_t->iFlag = W_LAST + (pthData->pInputData->iOut * 10);
    
    /* Printing some end information */
    if (write_output_line(pWriter_t)==ERROR)
    {
        thread_flag_post(&pthData->uiFlags, TH_ERROR);
        return NULL;
    }
    /* We done here, Everyone needs to know about it */
    thread_flag_post(&pthData->uiFlags,TH_MAX_LINES);
    writer_t_destroy(pWriter_t);
    return NULL;
}

/* init writer structure */
writer_t* writer_t_init(input_t *pInputData)
{
    if (pInputData==NULL){ return NULL; }
    
    writer_t *pWriter_t = malloc(sizeof(writer_t));
    
    pWriter_t->iFlag = 0 + (pInputData->iOut * 10);
    
    if (pWriter_t->iFlag>0)
    {
        if ((pWriter_t->file = fopen("Result.txt", "w+"))==NULL)
        {
            printf("Cannot create Result.txt file" );
            free(pWriter_t);
            return NULL;
        }
    }
    else pWriter_t->file = NULL;
    
    pWriter_t->szMask = pInputData->szMask;
    pWriter_t->szSeparator = pInputData->szSeparator;
    pWriter_t->iAmaunt = 0;
    pWriter_t->pnTemp = NULL;
    return pWriter_t;
}

/* Destroy file writer structure */
void writer_t_destroy(writer_t* pWriter_t)
{
    if (pWriter_t->file!=NULL)
    {
        fclose(pWriter_t->file);
        pWriter_t->file = NULL;
    }
    pWriter_t->szMask = NULL;
    pWriter_t->szSeparator = NULL;
    if (pWriter_t->pnTemp!=NULL)
    {
        destroy_node(pWriter_t->pnTemp);
        pWriter_t->pnTemp = NULL;
    }
    free(pWriter_t);
    pWriter_t = NULL;
}

/* Writer output switch*/
int write_output_line(writer_t *pWriter_t)
{
    if ((pWriter_t->iFlag) <10)
    {
        return show_output_on_console(pWriter_t);
    }
    else
    {
        return write_output_in_file(pWriter_t);
    }
}

/* Printing the output lines in console */
int show_output_on_console(writer_t *pWriter_t)
{
    if (pWriter_t==NULL)
    {
        return ERROR;
    }
    switch (pWriter_t->iFlag%10)
    {
        case W_START:
            if (pWriter_t->szMask!=NULL)
            {
                printf("Lines coresponding to mask %s: \n",pWriter_t->szMask);
                (pWriter_t->iFlag)++;
                break;
            }
            else return ERROR;
        case W_FIRST:
            if (pWriter_t->pnTemp!=NULL && pWriter_t->pnTemp->szLine!=NULL)
            {
                printf("%s",pWriter_t->pnTemp->szLine);
                (pWriter_t->iFlag)++;
                (pWriter_t->iAmaunt)++;
                break;
            }
            else return ERROR;
        case W_CURENT:
            if (pWriter_t->pnTemp!=NULL && pWriter_t->pnTemp->szLine!=NULL && pWriter_t->szSeparator!=NULL)
            {
                printf("%s%s",pWriter_t->szSeparator,pWriter_t->pnTemp->szLine);
                (pWriter_t->iAmaunt)++;
                break;
            }
            else return ERROR;
        case W_LAST:
            if (pWriter_t->szMask!=NULL)
            {
                if ((pWriter_t->iAmaunt)==0)
                {
                    printf("Log file Readed. \n");
                    printf("There is no lines corresponding to '%s' \n",pWriter_t->szMask);
                    break;
                }
                else
                {
                    printf("\nLog file Readed. \n");
                    printf("Totall amount of lines coresponding to '%s': %d\n",
                           pWriter_t->szMask, pWriter_t->iAmaunt);
                    break;
                }
            } else return ERROR;
            
        default:
            return ERROR;
    }
    
    return SUCCESS;
}

/* Writing the output lines in file */
int write_output_in_file(writer_t *pWriter_t)
{
    if (pWriter_t==NULL)
    {
        return ERROR;
    }
    switch (pWriter_t->iFlag%10)
    {
        case W_START:
            if (pWriter_t->szMask!=NULL && pWriter_t->file!=NULL)
            {
                fprintf(pWriter_t->file,"Lines coresponding to mask %s: \n",pWriter_t->szMask);
                printf("Lines coresponding to mask %s in Resulst.txt \n",pWriter_t->szMask);
                
                (pWriter_t->iFlag)++;
                break;
            }
            else return ERROR;
        case W_FIRST:
            if (pWriter_t->pnTemp!=NULL && pWriter_t->file!=NULL)
            {
                fprintf(pWriter_t->file,"%s",pWriter_t->pnTemp->szLine);
                (pWriter_t->iFlag)++;
                (pWriter_t->iAmaunt)++;
                break;
            }
            else return ERROR;
        case W_CURENT:
            if (pWriter_t->pnTemp!=NULL && pWriter_t->szSeparator!=NULL && pWriter_t->file!=NULL)
            {
                fprintf(pWriter_t->file,"%s%s",pWriter_t->szSeparator,pWriter_t->pnTemp->szLine);
                (pWriter_t->iAmaunt)++;
                break;
            }
            else return ERROR;
        case W_LAST:
            if (pWriter_t->szMask!=NULL)
            {
                printf("Log file Readed, Corresponding lines in Result.txt\n");
                if ((pWriter_t->iAmaunt)==0)
                {
                    fprintf(pWriter_t->file,"There is no lines corresponding to '%s' \n",pWriter_t->szMask);
                    printf("There is no lines corresponding to '%s' \n",pWriter_t->szMask);
                    break;
                }
                else
                {
                    fprintf(pWriter_t->file,"\nTotall amount of lines coresponding to '%s': %d\n", pWriter_t->szMask, pWriter_t->iAmaunt);
                    printf("Totall amount of lines coresponding to '%s': %d\n", pWriter_t->szMask, pWriter_t->iAmaunt);
                    break;
                }
            } else return ERROR;
        default:
            return ERROR;
    }
    return SUCCESS;
}
