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
#define ERROR   -1
#define SUCCESS  0
#define YES      1
#define NO       1

/* Standart path to log file for testing */
#define DEFAULT_FILE_PATH "log.txt"

/* Global mutexes: 1 - read->search mutex, 2 - search->write mutex 3 - common parameters mutex  */
pthread_mutex_t mutex_RS, mutex_SW, mutex_CP;
int file_end = 0, max_lines = 0;


/* ./execute -f-file_path -m-mask -l-max_lines -t-scan_tail -s-separator */

/* One direction linked list for output buffer */
typedef struct list
{
    /* Buffer for line */
    char* string;
    /* pointer to next element */
    struct list *pstNext;
} list_t;

/* Struct for threads */
typedef struct data
{
    const char *szMask, *szFilePath, *szSeparator;
    int iScanTail, iMaxLines, iTotallStrings;
    char szLine_for_search[1024],szLine_for_write[1024];
    list_t *pstOutBuffer;
    int file_end, max_lines,line_to_search,line_to_write;
    
    
} data_t;

/* Does the threads still needs to work */
int NeedMore(){
    pthread_mutex_lock(&mutex_CP);
    if ( (file_end != 0) || (max_lines != 0) )
    {
        pthread_mutex_unlock(&mutex_CP);
        return NO;
    }
    pthread_mutex_unlock(&mutex_CP);
    return YES;
}

/* Adding new element to the end of list */
void list_add_end(list_t *pstHead, char *string)
{
    /*While it's not the end*/
    while (pstHead->pstNext!=NULL)
    {
        pstHead=pstHead->pstNext;
    }
    
    /* create new list element */
    list_t *pstTemp = NULL;
    pstTemp = malloc(sizeof(list_t));
    pstTemp->string = malloc(strlen(string)+1);
    strcpy(pstTemp->string, string);
    pstTemp->pstNext=NULL;
    
    /* givin to the previous list element adress of the new list element */
    pstHead->pstNext = pstTemp;
}

/* Destroing list */
void list_destroy(list_t *pstHead)
{
    /* Backup adress */
    list_t *pstTemp = NULL;
    /* Untill we have something */
    while (pstHead!=NULL)
    {
        pstTemp = pstHead->pstNext;
        free(pstHead->string);
        pstHead->string=NULL;
        free(pstHead);
        pstHead = pstTemp;
    }
}

/* Print buffer to console */
void list_printing(list_t *pstHead, char *Szseparator)
{
    /* Print the Title of list */
    printf("%s",pstHead->string);
    pstHead=pstHead->pstNext;
    /* Until we have something to print */
    while (pstHead!=NULL)
    {
        /* If we at the end of list */
        if (pstHead->pstNext==NULL) {
            printf("%s \n", pstHead->string);
            pstHead=pstHead->pstNext;
        }
        else{
            printf("%s%s",pstHead->string,Szseparator);
            pstHead=pstHead->pstNext;
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
    if ((file = fopen(pstData->szFilePath, "r")) == NULL)
    {
        /* File cannot be oppened */
        printf("File cannot be oppened");
        return NULL;
    }
    /* if file opened  */
    else
    {
        /* scan file from tail? */
        if (pstData->iScanTail==1)
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
                if (pstData->max_lines == 1)
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
                        strcpy(pstData->szLine_for_search,szLine);
                        
                        
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
                if (pstData->max_lines == 1)
                {
                    break;
                }
                szLine[i] = (char)iTempForChar;
                if (iTempForChar=='\n')
                {
                    
                    szLine[i]='\0';
                    /* Give a signal to other thread for searching */
                    strcpy(pstData->szLine_for_search,szLine);
                    i = 0;
                }
                else
                {
                    i++;
                }
            }
        }
        
    }
    pstData->file_end = 1;
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
    main_data.iTotallStrings = 0;
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
