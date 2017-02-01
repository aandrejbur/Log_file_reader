/* Include system librarys */
#include <pthread.h>

/* Include own librarys */
#include "list_t.h"
#include "usefull_utilities.h"

/* ./execute file_path -mask max_lines scan_tail output(1-file, 0-console) separator */

/* Block size to read = 4MB */
#define BLOCK_SIZE 4194304
#define BIGEST_LINE 2048

/* Codes to end program */
#define FILE_END    10
#define MAX_LINES   20

/* The answer to main question */
#define STOP        42

/* Input parameters */
char *szMask, *szFilePath, *szSeparator;
int iScanTail, iMaxLines, iAmount, iOut;

/* Struct for threads */
typedef struct comon_data
{
    /* Queues for search and write */
    list_t *plSearchQueue, *plWrightQueue;
    /* Flags for stoping the process */
    int iFile_end, iMax_lines, Search_is_done, iError;
    /* Global mutexes: 1 - read->search mutex, 2 - search->write mutex, 3 - common parameters mutex  */
    pthread_mutex_t mutex_RS, mutex_SW, mutex_CP;
} comon_data_t;

/* Search for mask in string, Return 0 if finded*/
int SearchForMaskInString(char * szString, regex_t *preg)
{
    return regexec(preg, szString, 0, NULL, REG_EXTENDED);
}

/* Parcing input parammeters */
int start_patameters_parsing(const char* argv)
{
    //char szLineTemp[512];
    if (argv == NULL)
    {
        return ERROR;
    }
    else
    {
        //strcat(szLineTemp,argv);
        if (argv[0]!= '-')
        {
            return ERROR;
        }
        else
        {
            switch ((argv[1]))
            {
                case 'f':
                    szFilePath = malloc(strlen(argv));
                    sprintf(szFilePath,"%s",argv+3);
                    //memmove(szFilePath, argv+3,strlen(argv)-2);
                    break;
                case 'm':
                    szMask = malloc(strlen(argv));
                    sprintf(szMask,"%s",argv+3);
                    //memmove(szMask, argv+3,strlen(argv)-2);
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
                    //memmove(szSeparator, argv+3,strlen(argv)-2);
                    break;
                case 'h':
                    printf("Programm call syntax: ./log_reader -f='FILE_PATH' -m='MASK' -c='MAX LINES'(default 10000) \n");
                    printf("                      -d='1(0)'File scan direction: -d=1 - from tail\n");
                    printf("                                                    -d=2 - from the begining (default) \n");
                    printf("                      -o='1(0)' output type: -o=1 - Save output data in file: 'Result.txt' \n");
                    printf("                                             -o=0 - Print output data on screen \n");
                    printf("                      -s='SEPARATOR' default separator: '\\n' \n");
                    printf("                      -h - help \n");
                    return SUCCESS;
                default:
                    break;
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
    int i = 0, j = 0, direction = 0, search_can_start = 0, counter = 0;
    FILE *file;
    long lFSize = 0, lBytesToReadOnce = 0;
    char szLine[BIGEST_LINE], *Block = NULL;
    
    /* try to open file for reading */
    if ( (file = fopen((const char*)szFilePath, "r")) == NULL )
    {
        /* File cannot be oppened */
        printf("Can't open the file \n");
        /* We are not alone, so lets inform everyone about it*/
        pthread_mutex_lock(&(pthData->mutex_CP));
        pthData->iError = 1;
        pthread_mutex_unlock(&pthData->mutex_CP);
        return NULL;
    }
    /* if file opened  */
    else
    {
        pthread_mutex_lock(&pthData->mutex_RS);
        direction = iScanTail;
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
                pthread_mutex_lock(&pthData->mutex_CP);
                if (pthData->iMax_lines != 0)
                {
                    pthData->iFile_end = 1;
                    pthread_mutex_unlock(&pthData->mutex_CP);
                    break;
                }
                else
                {
                    pthread_mutex_unlock(&pthData->mutex_CP);
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
                        
                        if (search_can_start == 0)
                        {
                            pthread_mutex_unlock(&pthData->mutex_RS);
                            search_can_start++;
                        }
                        /* Give a signal to other thread for searching */
                        pthread_mutex_lock(&pthData->mutex_RS);
                        /* Insert the new node in the end of the search queue */
                        list_tail_add(pthData->plSearchQueue, pnTemp);
                        /*Taking totall amount of nodes in search*/
                        counter =pthData->plSearchQueue->iNodes;
                        /* Unlock the mutex */
                        pthread_mutex_unlock(&pthData->mutex_RS);
                        j = 0;
                        if (counter>10000)
                        {
                            for (counter = 0; counter<100000; counter++)
                            {
                                counter++;
                                counter--;
                            }
                        }
                    }
                    else
                    {
                        j++;
                    }
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
            while (lFSize>=0)
            {
                /* Lets check that we still need to read */
                pthread_mutex_lock(&pthData->mutex_CP);
                if ((pthData->iMax_lines != 0) || (pthData->iError !=0))
                {
                    pthData->iFile_end = 1;
                    pthread_mutex_unlock(&pthData->mutex_CP);
                    break;
                }
                else
                {
                    pthread_mutex_unlock(&pthData->mutex_CP);
                }
                /* and now we will read and scan */
                fseek(file, -lBytesToReadOnce, SEEK_CUR);
                fread(Block, sizeof(char), lBytesToReadOnce, file);
                lFSize -=lBytesToReadOnce;
                /* Scaning block for lines */
                for (lI = lBytesToReadOnce-1; lI>=0; lI--)
                {
                    if ( (szLine[j] = Block[lI]) =='\n')
                    {
                        szLine[j]='\0';
                        array_swap(szLine, &j);
                        /* Creating a new node for this line */
                        pnTemp = node_init(szLine);
                        
                        if (search_can_start == 0)
                        {
                            pthread_mutex_unlock(&pthData->mutex_RS);
                            search_can_start++;
                        }
                        
                        /* Give a signal to other thread for searching */
                        pthread_mutex_lock(&pthData->mutex_RS);
                        /* Insert the new node in the end of the search queue */
                        list_tail_add(pthData->plSearchQueue, pnTemp);
                        /* Unlock the mutex */
                        pthread_mutex_unlock(&pthData->mutex_RS);
                        j = 0;
                    }
                    else
                    {
                        j++;
                    }
                }
                if (lFSize<=lBytesToReadOnce)
                {
                    lBytesToReadOnce = lFSize;
                }
            }
        }
        /* Give a signal to other thread that file is readed */
        pthread_mutex_lock(&pthData->mutex_CP);
        pthData->iFile_end = 1;
        pthread_mutex_unlock(&pthData->mutex_CP);
        /* Unlocking the mutex, if there was nothing to search */
        if (search_can_start==0)
        {
            pthread_mutex_unlock(&pthData->mutex_RS);
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
    regex_t preg;
    int search = 0, writer_can_work = 0;
    long lLength = 0;
    pthread_mutex_lock(&pthData->mutex_SW);
    
    lLength = strlen(szMask);
    char *szMask_temp = malloc(lLength+3);
    
    if (szMask[0]=='*')
    {
        sprintf(szMask_temp,"%s", szMask+1);
        lLength--;
    }
    else if(szMask[0]=='?')
    {
        sprintf(szMask_temp,"%s", szMask);
        szMask_temp[0]='.';
    }
    else
    {
        sprintf(szMask_temp," %s", szMask);
        lLength++;
    }
    if (szMask_temp[lLength-1]=='*')
    {
        szMask_temp[lLength-1]=szMask_temp[lLength];
        szMask_temp[lLength] = '\0';
        
    }
    else if(szMask_temp[lLength-1]=='?')
    {
        szMask_temp[lLength-1] = '.';
    }
    else
    {
        szMask_temp[lLength]=' ';
        szMask_temp[lLength]='\0';
        lLength++;
    }
    //strncat(szMask_temp, szMask, strlen(szMask));
    /* Compiling regular expression */
    //iLength = strlen(szMask_temp);
    if ((regcomp(&preg, szMask_temp, 0))!=0 )
    {
        /* Cannot compile regular expression */
        printf("Cannot compile regular expression");
        pthread_mutex_lock(&pthData->mutex_CP);
        pthData->iError = 1;
        pthread_mutex_unlock(&pthData->mutex_CP);
        if (writer_can_work==0)
        {
            pthread_mutex_unlock(&pthData->mutex_SW);
            printf("Searcher give the right to writer to work 0 -end \n");
            writer_can_work++;
        }
        return NULL;
    }
    while (1)
    {
        /* Try to get the line from the search queue */
        pthread_mutex_lock(&pthData->mutex_RS);
        if (pthData->plSearchQueue->iNodes > 0)
        {
            search++;
            pnTemp=get_node_top(pthData->plSearchQueue);
        }
        pthread_mutex_unlock(&pthData->mutex_RS);
        /* If we have the line for search */
        if (search>0)
        {
            /* let's search in the line */
            if (SearchForMaskInString(pnTemp->szLine,&preg) == 0)
            {
                if (writer_can_work == 0)
                {
                    printf("Searcher give the right to writer to work 1 -end \n");
                    pthread_mutex_unlock(&pthData->mutex_SW);
                    writer_can_work++;
                }
                /* if we found something */
                pthread_mutex_lock(&pthData->mutex_SW);
                /* send the line to writer queue */
                list_tail_add(pthData->plWrightQueue, pnTemp);
                pnTemp = NULL;
                pthread_mutex_unlock(&pthData->mutex_SW);
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
            /* a check if the file have ended or we have all needed lines */
            pthread_mutex_lock(&pthData->mutex_CP);
            if ( (pthData->iFile_end!=0) || (pthData->iMax_lines!=0) || (pthData->iError!=0))
            {
                pthData->Search_is_done = 1;
                pthread_mutex_unlock(&pthData->mutex_CP);
                if (writer_can_work==0)
                {
                    printf("Searcher give the right to writer to work 4 -end \n");
                    pthread_mutex_unlock(&pthData->mutex_SW);
                    writer_can_work++;
                }
                break;
            }
            else
            {
                pthread_mutex_unlock(&pthData->mutex_CP);
            }
        }
    }
    /* Unlocking the Search->write mutex if there was nothing to write */
    if (writer_can_work==0)
    {
        printf("Searcher give the right to writer to work 3 -end \n");
        pthread_mutex_unlock(&pthData->mutex_SW);
        writer_can_work++;
    }
    printf("Searcher -end \n");
    free(szMask_temp);
    regfree(&preg);
    return NULL;
}
/* Function for writing thread */
void* writer_thread(void *pThreadData)
{
    comon_data_t *pthData = (comon_data_t*)pThreadData;
    int write = 0, max_lines = 0, first = 0;
    node_t *pnTemp = NULL;
    FILE *file = NULL;
    if (iOut!=0) {
        file = fopen("Result.txt", "w+");
        fprintf(file,"Lines coresponding to mask '%s': \n",szMask);
        printf("Output data in Result.txt \n");
    }
    else{
        printf("Lines coresponding to mask '%s': \n",szMask);
    }
    max_lines = iMaxLines;
    while (1)
    {
        /* lets try to get a line for our work */
        pthread_mutex_lock(&pthData->mutex_SW);
        if (pthData->plWrightQueue->iNodes > 0)
        {
            write++;
            pnTemp=get_node_top(pthData->plWrightQueue);
        
        }
        pthread_mutex_unlock(&pthData->mutex_SW);
        /* If we have something to write */
        if (write>0)
        {
            if (first == 0)
            {
                if (iOut!=0)
                {
                    fprintf(file,"%s",pnTemp->szLine);
                }
                else{
                    printf("%s",pnTemp->szLine);
                }
                first++;
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
            write--;
            destroy_node(pnTemp);
            pnTemp = NULL;
        }
        /* Are we have every line we need? */
        if (max_lines == 0)
        {
            /* Everyone need\s to know about it */
            pthread_mutex_lock(&pthData->mutex_CP);
            pthData->iMax_lines = 1;
            pthread_mutex_unlock(&pthData->mutex_CP);
            break;
        }
        /* Is there anyone else still working */
        if (write == 0)
        {
           /* We need to check, if something stil in search or read */
           pthread_mutex_lock(&pthData->mutex_CP);
           if ( (pthData->iError!=0) || ( (pthData->iFile_end!=0)&&(pthData->Search_is_done!=0)) )
           {
               pthData->iMax_lines = 1;
               pthread_mutex_unlock(&pthData->mutex_CP);
               break;
           }
           else
           {
               pthread_mutex_unlock(&pthData->mutex_CP);
           }
        }
    }
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
    printf("Writer -end \n");
    return NULL;
}

int main(int argc, const char * argv[])
{
    /* input parametters checking */
    if (argc==1)
    {
        printf("Start without parammeters create 2GB File 'Programmer_Commandments.txt' \n");
        //file_create(8500000);
        //return SUCCESS;
        /* Test parameters set for xcode */
        szFilePath = "Programmer_Commandments.txt"; szMask = "*untu*\0";
        iMaxLines    = 10;
        iScanTail    = 0;
        iOut = 1;
        szSeparator  = "\n";
    }
    else {
        int i = 0;
        for (i = 1; i<argc; i++) {
            if (start_patameters_parsing(argv[i])!= SUCCESS)
            {
                /* Print error: not enaugh input parameters */
                printf("Input parammeters error \n");
                return ERROR;
            }
        }
        //szFilePath = argv[1]; szMask = argv[2];
        if ( !iMaxLines ){ iMaxLines = 10000;     }
        if ( !iScanTail ){ iScanTail = 0;         }
        /* Default output - screen */
        if ( !iOut ){ iOut =  0;                  }
        if ( !szSeparator ){ szSeparator  = "\n\0"; }
    }
    /* Common data structure for threads */
    comon_data_t stThreadData = {list_init(), list_init(), 0,0,0,0};

    /* Initialisation of mutexes */
    pthread_mutex_init(&stThreadData.mutex_SW, NULL);
    pthread_mutex_init(&stThreadData.mutex_RS, NULL);
    pthread_mutex_init(&stThreadData.mutex_CP,NULL);
    
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
    pthread_join(pthSearcher, NULL);
    pthread_join(pthWriter, NULL);

    /* Destroying mutexes */
    pthread_mutex_destroy(&stThreadData.mutex_RS);
    pthread_mutex_destroy(&stThreadData.mutex_SW);
    pthread_mutex_destroy(&stThreadData.mutex_CP);
    
    /* Destroying everything */
    list_destroy(stThreadData.plWrightQueue);
    list_destroy(stThreadData.plSearchQueue);
    //free(szFilePath);szFilePath=NULL;
    //free(szMask);szMask=NULL;
    
    if ( strcmp(szSeparator, "\n\0") != 0)
    {
        free(szSeparator);szSeparator=NULL;
    }
    return SUCCESS;
}
