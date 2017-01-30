#include <stdio.h>
#include <stdint.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


//#include <unistd.h>
//
//#include <stddef.h>
//#include <stdlib.h>
//#include <string.h>
//
//#include <pthread.h>
//

/* Exit codes */
#define ERROR       -1
#define SUCCESS     0
#define FILEEND     10
#define MAX_LINES   20

/* Standart path to log file for testing */
#define DEFAULT_FILE_PATH "log.txt"

/* Global mutexes: 1 - read->search mutex, 2 - search->write mutex 3 - common parameters mutex  */
pthread_mutex_t mutex_RS, mutex_SW;

/* Input parameters */
const char *szMask, *szFilePath, *szSeparator;
int iScanTail, iMaxLines, iTotallStrings;

/* ./execute -f-file_path -m-mask -l-max_lines -t-scan_tail -s-separator */


/* a node struct */
typedef struct node_t
{
    char* szLine;
    int iFlag;
    struct node_t *pnNext, *pnPrev;
} node_t;

/* Two direction linked list for output buffer */
typedef struct list
{
    int iNodes;
    char *pszTitle;
    /* pointers to next, previous, start and end nodes */
    struct node_t *pnHead, *pnTail;
} list_t;


/* Struct for threads */
typedef struct data
{
    
    /* Queue for search and write */
    list_t *plSearchQueue, *plWrightQueue;
    
    /* Output buffer */
    list_t *pstOutBuffer;
    
} data_t;

/* Add node_t to end of list_t */


/* Add node_t to start of list_t */
/* Destroy list_t*/
/* Print list_t*/



/* Adding new element to the end of the list_t */
void list_push(list_t *pstList, char *string, int flag)
{
    /* create new list element */
    node_t *pnNewNode = malloc(sizeof(node_t));
    pnNewNode->pnNext = NULL;
    pnNewNode->pnPrev = pstList->pnTail;
    pnNewNode->szLine = malloc(strlen(string)+1);
    strcpy(pnNewNode->szLine, string);
    pnNewNode->iFlag = flag;
    /* givin to the previous list element adress of the new list element */
    pstList->pnTail = pnNewNode;
    pstList->iNodes++;
}

/* Adding new node_t to the top of the list_t */
void list_pop(list_t *pstList, char *string, int flag)
{
    /* create new list element */
    node_t *pnNewNode = malloc(sizeof(node_t));
    pnNewNode->pnNext = pstList->pnHead;
    pnNewNode->pnPrev = NULL;
    pnNewNode->szLine = malloc(strlen(string)+1);
    strcpy(pnNewNode->szLine, string);
    pnNewNode->iFlag = flag;
    /* givin to the previous list element adress of the new list element */
    pstList->pnHead = pnNewNode;
    pstList->iNodes++;
}

/* deleting the element */
void delete_element(list_t *pstList, node_t *pnNode)
{
    /* if previous node_t exist */
    if (pnNode->pnPrev != NULL)
    {
        pnNode->pnPrev->pnNext = pnNode->pnNext;
        pnNode->pnNext = NULL;
    }
    /* if next node_t exist */
    if (pnNode->pnNext != NULL) {
        pnNode->pnNext->pnPrev = pnNode->pnPrev;
        pnNode->pnPrev = NULL;
    }
    pstList->iNodes--;
    free(pnNode->szLine);
    pnNode->szLine = NULL;
    free(pnNode);
    /* If in list was only one element */
    if (pstList->iNodes == 0)
    {
        pstList->pnHead = NULL;
        pstList->pnTail = NULL;
        free(pstList->pszTitle);
        pstList->pszTitle = NULL;
        free(pstList);
    }
}


/* Destroy list_t*/
void list_destroy(list_t *pstList)
{
    /* Backup adress of node */
    node_t *pnTemp = NULL;
    
    /* Untill we have something at the top of the list */
    while (pstList->pnHead != NULL)
    {
        /* Backup next node_t adress */
        pnTemp = pstList->pnHead->pnNext;
        /* if exist next note_t then updating his pointer to previous node_t */
        if (pstList->pnHead->pnNext!=NULL)
        {
            pstList->pnHead->pnNext->pnPrev = NULL;
        }
        /* Just in case */
        pstList->pnHead->pnPrev = NULL;
        /* Cleaning the next node_t pointer */
        pstList->pnHead->pnNext = NULL;
        /* Free the line with text */
        free(pstList->pnHead->szLine);
        pstList->pnHead->szLine = NULL;
        /* Free the head of the list */
        free(pstList->pnHead);
        /* Decrement the node_t counter */
        pstList->iNodes--;
        /* Go to the next node */
        pstList->pnHead = pnTemp;
        
    }
}

/* Print list_t to the console */
void list_printing(list_t *pstList, char *Szseparator)
{
    node_t *pnTemp = pstList->pnHead;
    /* Print the Title of list */
    printf("%s",pstList->pszTitle);
    
    /* Until we have something to print */
    while (pnTemp !=NULL)
    {
        /* If we at the end of list, we don't need to print the separator*/
        if (pnTemp->pnNext == NULL) {
            printf("%s \n", pnTemp->szLine);
            pnTemp=pnTemp->pnNext;
        }
        else{
            printf("%s%s",pnTemp->szLine,Szseparator);
            pnTemp=pnTemp->pnNext;
        }
    }
}




/* Search for mask in string */
int SearchForMaskInString(char * szString, regex_t *preg)
{
    return regexec(preg, szString, 0, NULL, 0);
}


/* Swoping the array */
void array_swap( char* array, int counter )
{
    char cTempC;
    int i;
    
    for (i = 0; i < counter/2; i++)
    {
        cTempC = array[i];
        (array[i]) = (char)(array[counter-1-i]);
        array[counter-1-i]=cTempC;
    }
}


/* Read file line by line */
void* read_file(void *pData)
{
    data_t *pstData = (data_t*)pData;
    FILE *file;
    int i = 0, iTempForChar = 0; long lFSize = 0;
    char szLine[1024];
    
    
    
    /* try to open file for reading */
    if ((file = fopen(szFilePath, "r")) == NULL)
    {
        /* File cannot be oppened */
        printf("File cannot be oppened");
        return NULL;
    }
    /* if file opened  */
    else
    {
        /* scan file from tail? */
        if (iScanTail==1)
        {
            /* Getting file size */
            fseek(file, 0, SEEK_END);
            lFSize = ftell(file);
            
            /* Setting up position for reading */
            fseek(file, -1, SEEK_END);
            
            /* Reading file rom end */
            while ( lFSize !=0)
            {
                /* Checking do we still need to work*/
                if (1)
                {
                    break;
                }
                if ((iTempForChar = fgetc(file)) != EOF )
                {
                    szLine[i]=(char)iTempForChar;
                    
                    /* If we find a line end */
                    if (iTempForChar=='\n')
                    {
                        szLine[i]='\0';
                        array_swap(szLine, i);
                        
                        /* Give a signal to other thread for searching */
                      //  strcpy(pstData->szLine_for_search,szLine);
                        pthread_mutex_lock(&mutex_RS);
                        // Adding new to list
                        
                        pthread_mutex_unlock(&mutex_RS);
                        
                        
                        //list_push(, <#char *string#>, <#int flag#>)
                        
                        
                        i=0;
                        lFSize--;
                        fseek(file, -2, SEEK_CUR);
                    }
                    else
                    {
                        fseek(file, -2, SEEK_CUR);
                        i++;
                        lFSize--;
                    }
                }
            }
        }
        else /* Read file from the beginning */
        {
            while ((iTempForChar = fgetc(file)) != EOF)
            {
                /* Checking do we still need to work*/
                
                szLine[i] = (char)iTempForChar;
                if (iTempForChar=='\n')
                {
                    
                    szLine[i]='\0';
                    /* Give a signal to other thread for searching */
                    
                    
                    //strcpy(pstData->szLine_for_search,szLine);
                    i = 0;
                }
                else
                {
                    i++;
                }
            }
        }
        
    }
    
    /*  */
    fclose(file);
    return NULL;
}

/* search for mask in rows */
void* search_in_row(void *pData)
{
    data_t *pstData = (data_t*)pData;
    regex_t preg;
    char szLine[1024];
    
    /* Compiling regular expression */
    if ((regcomp(&preg, pstData->szMask, 0))!=0 )
    {
        /* Cannot compile regular expression */
        printf("Cannot compile regular expression");
        return NULL;
    }
    
    while (1)
    {
        pthread_mutex_lock(mutex_RS);
        
        //
        
        if (plHead!= NULL) {
            
            
        }
        
        pthread_mutex_unlock(mutex_RS);
        
        
        strcpy(szLine,pstData->szLine_for_search );
        if ( (pstData->max_lines !=0) || (pstData->file_end !=0) )
        {
            break;
        }
        if (SearchForMaskInString(szLine,&preg)==0)
        {
            strcpy(pstData->szLine_for_write,szLine);
        }
    }
    regfree(&preg);
    return NULL;
}

/* write line to buffer */
void* write_to_buffer(void *pData)
{
    data_t *pstData = (data_t*)pData;
    
    while (1)
    {
        
        list_add_end(pstData->pstOutBuffer,pstData->szLine_for_write);
        pstData->iTotallStrings++;
        if (pstData->iTotallStrings==pstData->iMaxLines)
        {
            pstData->max_lines = 1;
            break;
        }
        if (pstData->file_end != 0) {
            break;
        }
    }
    return NULL;
}

int main(int argc, const char * argv[])
{
    data_t main_data;
    list_t *pstOutBuffer = NULL;
    pthread_t *pThreadReading = NULL, *pThreadSearching = NULL, *pThreadWriting = NULL;
    
    /* input parametters checking */
    if ((argc < 3) || (argc > 6))
    {
        /* Print error: not enaugh input parameters */
        printf("Incoorect input parameters \n");
        return ERROR;
    }
    
    /* Default parammeters set */
    iTotallStrings = 0;
    main_data.szFilePath = argv[1];
    main_data.szMask = argv[2];
    if (argv[3]) { main_data.iMaxLines = atoi(argv[3]);} else { main_data.iMaxLines = 10000; }
    if (argv[4]) { main_data.iScanTail = atoi(argv[4]);} else { main_data.iScanTail = 0; }
    if (argv[5]) { main_data.szSeparator = argv[5]; } else { main_data.szSeparator = "\n"; }
    main_data.max_lines = 0;
    main_data.file_end = 0;
    /* Starting the list with output strings */
    pstOutBuffer = malloc(sizeof(list_t));
    pstOutBuffer->string = malloc(128);
    sprintf(pstOutBuffer->string," Strings corresponding to mask - %s :\n", main_data.szMask);
    pstOutBuffer->pstNext=NULL;
    main_data.pstOutBuffer = pstOutBuffer;

    /* Trying to create threads */
    if (pthread_create(pThreadReading, NULL, &read_file, (void*)&main_data) != 0)
    {
        printf("Cannot creat thread for file reader");
        return ERROR;
    }
    if (pthread_create(pThreadSearching, NULL, &read_file, (void*)&main_data) != 0)
    {
        printf("Cannot creat thread for search");
        return ERROR;
    }
    if (pthread_create(pThreadWriting, NULL, &read_file, (void*)&main_data) != 0)
    {
        printf("Cannot creat thread for write");
        return ERROR;
    }
    
    /* wait until all threads are stopped */
    pthread_join(*pThreadReading, NULL);
    pthread_join(*pThreadSearching, NULL);
    pthread_join(*pThreadWriting, NULL);
    
    /* printing the result and destroy everithing */
    list_printing(main_data.pstOutBuffer, (char*)main_data.szSeparator);
    list_destroy(main_data.pstOutBuffer);
    return SUCCESS;
}
