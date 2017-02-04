/* Include system libraries */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/* Include own libraries */
#include "list_t.h"
#include "usefull_utilities.h"
#include "search_lib.h"

/* Block size to read = 4MB */
#define BLOCK_SIZE 4194304
#define BIGEST_LINE 4096

#define HELP 911

/* Input parameters */
char *szMask, *szFilePath, *szSeparator;
int iScanTail, iMaxLines, iAmount, iOut;

/* Global mutexes: 1 - read->search mutex, 2 - search->write mutex, 3 - common parameters mutex  */
pthread_mutex_t mutex_RS, mutex_SW, mutex_CP;

/* A semaphore for writing */
sem_t* semafor_SW;

/* Struct for threads */
typedef struct comon_data
{
    /* Queues for search and write */
    list_t *plSearchQueue, *plWrightQueue;
    /* Flags for stoping the process */
    int iFile_end, iMax_lines, Search_is_done, iError;
} comon_data_t;

void print_help()
{
    printf("Programm call syntax: ./log_reader -f='FILE_PATH' -m='MASK' -c='MAX LINES'(default 10000) \n");
    printf("                      -d='1(0)'File scan direction: -d=1 - from tail\n");
    printf("                                                    -d=2 - from the begining (default) \n");
    printf("                      -o='1(0)' output type: -o=1 - Save output data in file: 'Result.txt' \n");
    printf("                                             -o=0 - Print output data on screen \n");
    printf("                      -s='SEPARATOR' default separator: '\\n' \n");
    printf("                      -h - help \n");
}

/* Parse input parammeters */
int start_patameters_parsing(const char* argv)
{
    if (argv == NULL)
    {
        printf("Option incorrect\n");
        return ERROR;
    }
    else
    {
        //strcat(szLineTemp,argv);
        if (argv[0]!= '-')
        {
            printf("Each option must start with '-' symbol!!!\n");
            return ERROR;
        }
        else
        {
            switch ((argv[1]))
            {
                case 'f':
                    szFilePath = malloc(strlen(argv));
                    sprintf(szFilePath,"%s",argv+3);
                    break;
                case 'm':
                    szMask = malloc(strlen(argv));
                    sprintf(szMask,"%s",argv+3);
                    break;
                case 'c':
                    iMaxLines = atoi((argv+3));
                    break;
                case 'd':
                    iScanTail = atoi((argv+3));
                    break;
                case 'o':
                    iOut = atoi((argv+3));
                    break;
                case 's':
                    szSeparator = malloc(strlen(argv));
                    sprintf(szSeparator,"%s",argv+3);
                    break;
                case 'h':
                    return HELP;
                default:
                    printf("Incorrect option!\n");
                    return ERROR;
            }
        }
    }
    return SUCCESS;
}

/* Tread Functions: */
/* Function for file reader thread */
void* reader_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    node_t *pnTemp = NULL;
    int i = 0, j = 0, iCounter = 0,iSearchCanWork = 0;
    FILE *file;
    long lFSize = 0, lBytesToReadOnce = 0, lI = 0;
    char szLine[BIGEST_LINE], *Block = NULL;
    
    /* try to open file for reading */
    if ( (file = fopen((const char*)szFilePath, "r")) == NULL )
    {
        /* File cannot be oppened */
        printf("Can't open the file \n");
        /* We are not alone, so lets inform everyone about it*/
        pthread_mutex_lock(&mutex_CP);
        pthData->iError = 1;
        pthread_mutex_unlock(&mutex_CP);
        return NULL;
    }
    /* if file opened  */
    else
    {
        /* Lock the mutex so the search thread will have to wait */
        pthread_mutex_lock(&mutex_RS);
        
        /* Getting file size */
        fseek(file, 0, SEEK_END);
        lFSize = ftell(file);
        
        /* We determine how many bytes we read at a time */
        if (lFSize>BLOCK_SIZE)
        {
            Block = malloc(BLOCK_SIZE +1);
            lBytesToReadOnce = BLOCK_SIZE;
        }
        else
        {
            lBytesToReadOnce = lFSize;
            Block = malloc(lFSize +1 );
        }
        
        /* Where do we need to start */
        if (iScanTail == 0)
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
                    /* If we have all the lines we need */
                    pthData->iFile_end = 1;
                    pthread_mutex_unlock(&mutex_CP);
                    
                    /* Stop the reader */
                    break;
                }
                else
                {
                    pthread_mutex_unlock(&mutex_CP);
                }
                
                /* and now we will read and scan */
                fread(Block, sizeof(char), lBytesToReadOnce, file);
                lFSize -= lBytesToReadOnce;
                
                /* Scaning block for lines */
                for (i = 0; i<lBytesToReadOnce; i++)
                {
                    if ( (szLine[j] = Block[i]) =='\n')
                    {
                        szLine[j]='\0';
                       
                        /* Creating a new node for this line */
                        pnTemp = node_init(szLine);
                        
                        /* Checking and unleashing the searcher thread*/
                        if (iSearchCanWork == 0)
                        {
                            pthread_mutex_unlock(&mutex_RS);
                            iSearchCanWork++;
                        }
                        
                        /* Transfer a new node to the search queue */
                        pthread_mutex_lock(&mutex_RS);
                        
                        /* Insert the new node in the end of the search queue */
                        list_tail_add(pthData->plSearchQueue, pnTemp);
                        /*Taking totall amount of nodes in search*/
                        iCounter =pthData->plSearchQueue->iNodes;
                        
                        /* Unlock the mutex */
                        pthread_mutex_unlock(&mutex_RS);
                        
                        /* position in line to 0 */
                        j = 0;
                        
                        /* Give a break to the search thread, if there is too many nodes in queue */
                        if (iCounter>20000)
                        {
                            usleep(1000);
                        }
                    }
                    else
                    {
                        /* increment line position */
                        j++;
                    }
                }
                /* if unread file size is smaller than a block */
                if (lFSize<=lBytesToReadOnce)
                {
                    lBytesToReadOnce = lFSize;
                }
            }
        }
        else
        {
            /* Scaning from the end of file */
            j = 0;
            long iPosition = 0;
            while (1)
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

                iPosition += lBytesToReadOnce;
                fseek(file, -iPosition, SEEK_END);
                fread(Block, sizeof(char), lBytesToReadOnce, file);
                lFSize -= lBytesToReadOnce;
                
                /* Scaning block for lines */
                for (lI = lBytesToReadOnce-1; lI>=0; lI--)
                {
                    if ( (szLine[j] = Block[lI]) =='\n')
                    {
                        szLine[j]='\0';
                        array_swap(szLine, &j);
                       
                        /* Creating a new node for this line */
                        pnTemp = node_init(szLine);
                        
                        /* Checking and unleashing the searcher thread*/
                        if (iSearchCanWork == 0)
                        {
                            pthread_mutex_unlock(&mutex_RS);
                            iSearchCanWork++;
                        }
                        
                        /* Give a signal to other thread for searching */
                        pthread_mutex_lock(&mutex_RS);
                        
                        /* Insert the new node in the end of the search queue */
                        list_tail_add(pthData->plSearchQueue, pnTemp);
                        
                        /*Taking totall amount of nodes in search*/
                        iCounter =pthData->plSearchQueue->iNodes;
                        
                        /* Unlock the mutex */
                        pthread_mutex_unlock(&mutex_RS);
                        j = 0;
                        
                        /* Give a break to the search thread, if there is too many nodes in queue */
                        if (iCounter>20000)
                        {
                            usleep(1000);
                            
                        }
                    }
                    else
                    {
                        j++;
                    }
                }
                if (lFSize <= lBytesToReadOnce)
                {
                    lBytesToReadOnce = lFSize;
                }
                
                if (lFSize==0)
                {
                    /* Sending the last line */
                    
                    array_swap(szLine, &j);
                    szLine[j+1] = '\0';
                    pnTemp = node_init(szLine);
                    /* Give a signal to other thread for searching */
                    pthread_mutex_lock(&mutex_RS);
                    
                    /* Insert the new node in the end of the search queue */
                    list_tail_add(pthData->plSearchQueue, pnTemp);
                    
                    /*Taking totall amount of nodes in search*/
                    iCounter =pthData->plSearchQueue->iNodes;
                    /* Unlock the mutex */
                    pthread_mutex_unlock(&mutex_RS);
                    break;
                }
            }
        }
        
        /* Give a signal to other thread that file is readed */
        pthread_mutex_lock(&mutex_CP);
        pthData->iFile_end = 1;
        pthread_mutex_unlock(&mutex_CP);
        
        /* Unlocking the mutex, if there was nothing to search */
        if (iSearchCanWork==0)
        {
            pthread_mutex_unlock(&mutex_RS);
        }
        
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
    //regex_t preg;
    int iSearch = 0;
    long lLength = 0;
    
    search_t *psSearch = NULL;
    
    /* Creating a simple regular expression */
    lLength = strlen(szMask);
    //char *szMask_temp = malloc(lLength+1);
    //sprintf(szMask_temp,"%s", szMask);
    psSearch = compile_search_expression(szMask);
    
    while (1)
    {
        /* Try to get the line from the search queue */
        pthread_mutex_lock(&mutex_RS);
        if (pthData->plSearchQueue->iNodes > 0)
        {
            iSearch=1;
            pnTemp=get_node_top(pthData->plSearchQueue);
        }
        else
        {
            iSearch = 0;
        }
        pthread_mutex_unlock(&mutex_RS);
        /* If we have the line for search */
        if (iSearch==1)
        {
            /* let's search in the line */
            //if (SearchForMaskInString(pnTemp->szLine,&preg) == 0)
            if (  psSearch->search(pnTemp->szLine, (void *)psSearch) == FOUND)
            {
                
                /* if we found something */
                pthread_mutex_lock(&mutex_SW);
                
                /* send the line to writer queue */
                list_tail_add(pthData->plWrightQueue, pnTemp);
                pnTemp = NULL;
                pthread_mutex_unlock(&mutex_SW);
                sem_post(semafor_SW);
                iSearch=0;
            }
            else
            {
                /* nothing we need is in this line, let's destroy it */
                destroy_node(pnTemp);
                pnTemp=NULL;
                iSearch=0;
            }
        }
        /* if we have nothing to do, lets check that we still need to work */
        else if (iSearch == 0)
        {
            /* a check if the file have ended or we have all needed lines */
            pthread_mutex_lock(&mutex_CP);
            if ( ((pthData->iFile_end!=0) || (pthData->iMax_lines!=0)) || (pthData->iError!=0))
            {
                /* There is nthing else we can do */
                pthData->Search_is_done = 1;
                pthread_mutex_unlock(&mutex_CP);
                break;
            }
            else
            {
                pthread_mutex_unlock(&mutex_CP);
            }
        }
    }
    sem_post(semafor_SW);
    //free(szMask_temp);
    return NULL;
}

/* Function for writer thread */
void* writer_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    int write = 0, max_lines = 0, first = 0;
    node_t *pnTemp = NULL;
    FILE *file = NULL;
    char BUFFER[BIGEST_LINE]="";
    
    /* Check the output*/
    if (iOut!=0)
    {
        file = fopen("Result.txt", "w+");
        setvbuf(file,BUFFER,_IOFBF,BIGEST_LINE);
        fprintf(file,"Lines coresponding to mask %s: \n",szMask);
        printf("Output data in Result.txt \n");
    }
    else{
        printf("Lines coresponding to mask %s: \n",szMask);
    }
    max_lines = iMaxLines;
    while (1)
    {
        
        sem_wait(semafor_SW);
        /* lets try to get a line for our work */
        pthread_mutex_lock(&mutex_SW);
        if (pthData->plWrightQueue->iNodes > 0)
        {
            write=1;
            pnTemp=get_node_top(pthData->plWrightQueue);
        
        }
        pthread_mutex_unlock(&mutex_SW);
        
        /* If we have something to write */
        if (write>0)
        {
            /*If it's a first line, we don't need separator*/
            if (first == 0)
            {
                if (iOut!=0)
                {
                    fprintf(file,"%s",pnTemp->szLine);
                }
                else{
                    printf("%s",pnTemp->szLine);
                }
                first=1;
            }
            else
            {
                if (iOut!=0)
                {
                    fprintf(file,"%s%s",szSeparator,pnTemp->szLine);
                }
                else
                {
                    printf("%s%s",szSeparator,pnTemp->szLine);
                }
            }
            max_lines--;
            write=0;
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
           /* We need to check, if something stil in search or read */
           pthread_mutex_lock(&mutex_CP);
           if (pthData->iError!=0)
           {
               pthData->iMax_lines = 1;
               pthread_mutex_unlock(&mutex_CP);
               break;
           }
           else if (pthData->Search_is_done!=0)
           {
               pthData->iMax_lines = 1;
               pthread_mutex_unlock(&mutex_CP);
               break;
           }
           else
           {
               pthread_mutex_unlock(&mutex_CP);
           }
        }
    }
    /* Counting amount of lines */
    iAmount = iMaxLines-max_lines;
    if (iOut!=0)
    {
        //file = fopen("Result.txt", "w+");
        fprintf(file,"Log file Readed succesfully, totall amount of lines coresponding to mask: %d\n", iAmount);
        fclose(file);
        printf("Log file Readed succesfully, totall amount of lines coresponding to mask: %d\n", iAmount);
    }
    else
    {
        /* Print that we succsesfully done */
        printf("\nLog file Readed succesfully, totall amount of lines coresponding to mask: %d\n", iAmount);
    }
    return NULL;
}

int main(int argc, const char * argv[])
{
    /* input parametters checking */
    if (argc==1)
    {
        //printf("Start without parammeters create 2GB File 'Programmer_Commandments.txt' \n");
        //file_create(8500000);
        szMask="?11*";
        szFilePath = "Programmer_Commandments.txt";
        iScanTail = 1;
        iMaxLines = 5;
        iOut = 0;
        szSeparator  = "\n\0";
        //return SUCCESS;
    }
    else {
        int i = 0;
        for (i = 1; i<argc; i++) {
            switch (start_patameters_parsing(argv[i])) {
                case HELP:
                    print_help();
                    return SUCCESS;
                case SUCCESS:
                    break;
                default:
                    return ERROR;
            }
        }
        if ((!szMask) || (!szFilePath))
        {
            /* Print error: not enaugh input parameters */
            printf("Input parammeters error: no file_path parammeters or mask parammeters \n");
            return ERROR;
        }
        if ( !iMaxLines ){ iMaxLines = 10000;     }
        if ( !iScanTail ){ iScanTail = 0;         }
        /* Default output - screen */
        if ( !iOut ){ iOut =  0;                  }
        if ( !szSeparator ){ szSeparator  = "\n\0"; }
    }
    
    /* Common data structure for threads */
    comon_data_t stThreadData = {list_init(), list_init(), 0,0,0,0};

    /* Initialisation of mutexes */
    pthread_mutex_init(&mutex_SW, NULL);
    pthread_mutex_init(&mutex_RS, NULL);
    pthread_mutex_init(&mutex_CP,NULL);
    
    /* Initialisation of writer semaphore */
    semafor_SW = sem_open("s1", O_CREAT, 0777, 0);
    
    if ((semafor_SW = sem_open("/semaphore", O_CREAT, 0644, 1)) == SEM_FAILED ) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    //if( sem_init(&semafor_SW,1,1) < 0)
    //{
//        perror("semaphore initilization");
//        exit(0);
    //}
    //semafor_SW = *sem_open("Semaphor for writer",0);
    
    /* Pointers to threads */
    pthread_t pthReader, pthSearcher, pthWriter;
    
    /* Trying to create threads */
    if (pthread_create(&pthReader, NULL, &reader_thread, (void* )&stThreadData) != 0)
    {
        printf("Cannot create thread for file reader");
        return ERROR;
    }
    if (pthread_create(&pthSearcher, NULL, &search_thread, (void*)&stThreadData) != 0)
    {
        printf("Cannot create thread for search");
        return ERROR;
    }
    if (pthread_create(&pthWriter, NULL, &writer_thread, (void*)&stThreadData) != 0)
    {
        printf("Cannot create thread for write");
        return ERROR;
    }
    
    /* wait until all threads are stopped */
    pthread_join(pthReader, NULL);
    pthread_join(pthSearcher, NULL);
    pthread_join(pthWriter, NULL);

    /* Destroying mutexes */
    pthread_mutex_destroy(&mutex_RS);
    pthread_mutex_destroy(&mutex_SW);
    pthread_mutex_destroy(&mutex_CP);
    
    /* Destroy semaphore */
    sem_close(semafor_SW);
    
    /* Destroying everything */
    list_destroy(stThreadData.plWrightQueue);
    list_destroy(stThreadData.plSearchQueue);
    
    //free(szFilePath);szFilePath=NULL; free(szMask);szMask=NULL;
    /* if it's not a default separator, we will need to free him*/
    //if ( strcmp(szSeparator, "\n\0") != 0)
    //{
//        free(szSeparator);szSeparator=NULL;
//    }
    return SUCCESS;
}
