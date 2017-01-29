#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define ERROR -1
#define SUCCESS 0

/* Standart path to log file for testing */
#define DEFAULT_FILE_PATH "log.txt"
/* ./execute -f-file_path -m-mask -l-max_lines -t-scan_tail -s-separator */

/* One direction list for output buffer */
typedef struct list{
    /* Buffer for line */
    char string[1024];
    /* pointer to next element */
    struct list *pstNext;
} list_t;

/* Adding new element to the end of list */
void list_add_end(list_t *pstHead, char *string)
{
    while (pstHead->pstNext!=NULL)
    {
        pstHead=pstHead->pstNext;
    }
    list_t *pstTemp = NULL;
    pstTemp = malloc(sizeof(list_t));
    sprintf(pstTemp->string, "%s", string);
    pstTemp->pstNext=NULL;
    pstHead->pstNext = pstTemp;
}

/* Destroing list */
void list_destroy(list_t *pstHead)
{
    list_t *pstTemp = NULL;
    while (pstHead!=NULL)
    {
        pstTemp = pstHead->pstNext;
        free(pstHead);
        pstHead = pstTemp;
    }
}

/* Print buffer to console */
void list_printing(list_t *pstHead, char *Szseparator)
{
    printf("%s",pstHead->string);
    pstHead=pstHead->pstNext;
    while (pstHead->pstNext!=NULL) {
        printf("%s%s",pstHead->string,Szseparator);
        pstHead=pstHead->pstNext;
    }
    printf("%s \n", pstHead->string);
}

/* Read line from the end of file */
int fread_line_tail(FILE *pf, int *position, char *szBuff)
{
    //fseek(<#FILE *#>, <#long#>, <#int#>)
    return SUCCESS;
}

/* Search for mask in string */
int SearchForMaskInString(char * szString, regex_t *preg)
{
    int retval = 0;
    if ( (retval = regexec(preg, szString, 0, NULL, 0)) == 0)
    {
        return 1;
    }
    return 0;
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

int main(int argc, const char * argv[])
{
    
    /* input parametters checking */
    if ((argc < 3) || (argc > 6))
    {
        /* Print error: not enaugh input parameters */
        printf("Incoorect input parameters \n");
        return ERROR;
    }
    
    const char *szMask, *szFilePath, *szSeparator;
    
    regex_t preg;
    int iScanTail = 0, iMaxLines = 1000, iTotallStrings;
    int iTempForChar = 0;
    char szLine[1024]={0};
    FILE *file;
    list_t *pstOutBuffer = malloc(sizeof(list_t));
    
    /* Default parammeters read */
    iTotallStrings = 0;
    szFilePath = argv[1];
    szMask = argv[2];
    if (argv[3]) { iMaxLines = atoi(argv[3]);}
    if (argv[4]) { iScanTail = atoi(argv[4]);}
    if (argv[5]) { szSeparator = argv[5]; } else { szSeparator = "\n"; }
    
    /* Compiling regular expression */
    if ((regcomp(&preg, szMask, 0))!=0 )
    {
        /* Cannot compile regular expression */
        printf("Cannot compile regular expression");
        return ERROR;
    }
    
    /* Starting the list with output strings */
    sprintf(pstOutBuffer->string," Strings corresponding to mask - %s :\n", szMask);
    pstOutBuffer->pstNext=NULL;
    
    /*  */
    if ((file = fopen(szFilePath, "r")) == NULL)
    {
        /* File cannot be oppened */
        printf("File cannot be oppened");
        return ERROR;
    }
    
    int i = 0;
    long iFSize = 0;
    
    if (iScanTail==1)
    {
        char szLineFromEnd[1024];
        fseek(file, 0, SEEK_END);
        iFSize = ftell(file);
        fseek(file, -1, SEEK_END);
        while ( iFSize !=0)
        {
            if ((iTempForChar = fgetc(file)) != EOF )
            {
                szLineFromEnd[i]=(char)iTempForChar;
                if (iTempForChar=='\n')
                {
                    szLineFromEnd[i]='\0';
                    array_swap(szLineFromEnd, i);
                    if (SearchForMaskInString(szLineFromEnd,&preg)==1)
                    {
                        list_add_end(pstOutBuffer,szLineFromEnd);
                        iTotallStrings++;
                    
                    }
                    i=0;
                    iFSize--;
                    fseek(file, -2, SEEK_CUR);
                }
                else
                {
                    fseek(file, -2, SEEK_CUR);
                    i++;
                    iFSize--;
                }
            }
        }
    }
    else
    {
        while ((iTempForChar = fgetc(file)) != EOF)
        {
            szLine[i] = (char)iTempForChar;
            if (iTempForChar=='\n')
            {
                if (SearchForMaskInString(szLine,&preg)==1)
                {
                    iTotallStrings++;
                    szLine[i]='\0';
                    list_add_end(pstOutBuffer,szLine);
                }
                memset(szLine, 0, sizeof(szLine));
                i = 0;
            }
            else
            {
                
                i++;
            }
        }
    }
    
    list_printing(pstOutBuffer, (char *)szSeparator);
    list_destroy(pstOutBuffer);
    fclose(file);
    regfree(&preg);
    return SUCCESS;
}
