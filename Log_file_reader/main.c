/* Include system libraries */
#include <unistd.h>
#include <time.h>

/* Include own libraries */
#include "includes.h"

/** Global mutexes */
extern pthread_mutex_t mutex_RS, mutex_SW, mutex_CP;
/* Global semaphores */
extern sem_t *semaphor_RS, *semaphor_SW, *semaphor_CP;

/* default parameters set */
void default_setup(input_t *pInputData)
{
    if ( 0 == pInputData->iMaxLines )
    {
        pInputData->iMaxLines = 1000;
    }
    if ( pInputData->szSeparator == NULL )
    {
        pInputData->szSeparator  = "\n";
    }
}

/* Parse input parammeters */
int start_parameters_parsing(const char* argv,input_t *pInputData)
{
    if (argv == NULL)
    {
        printf("Option incorrect\n");
        return ERROR;
    }
    else
    {
        if (argv[0]!= '-')
        {
            printf("Each option must start with '-' symbol!!!\n");
            return ERROR;
        }
        else if (argv[1]=='h')
        {
            return HELP;
        }
        else if ((argv[2]=='\0')||(argv[3]=='\0'))
        {
            printf("Incorrect option!\n");
            return ERROR;
        }
        else
        {
            switch ((argv[1]))
            {
                case 'f': pInputData->szFilePath = malloc(strlen(argv)-2);
                    sprintf(pInputData->szFilePath, "%s", argv+3);
                    break;
                case 'm': pInputData->szMask = malloc(strlen(argv)-2);
                    sprintf(pInputData->szMask, "%s", argv+3);
                    break;
                case 's': pInputData->szSeparator = malloc(strlen(argv)-2);
                    sprintf(pInputData->szSeparator, "%s", argv+3);
                    break;
                case 'c': pInputData->iMaxLines  = atoi((argv+3));  break;
                case 'd': pInputData->iScanTail  = atoi((argv+3));  break;
                case 'o': pInputData->iOut       = atoi((argv+3));  break;
                default:
                    printf("Incorrect option!\n");
                    return ERROR;
            }
        }
    }
    return SUCCESS;
}

/* Print help information */
void print_help()
{
    printf("Programm call syntax: ./log_reader\n");
    printf("Input parameters: \n");
    printf("        -f='FILE_PATH' - full path to log file\n");
    printf("        -m='MASK' - mask that we will search\n");
    printf("        -c='MAX LINES' - max lines number (default 10000) \n");
    printf("        -d='1(0)'File scan direction: -d=1 - from tail\n");
    printf("                                      -d=2 - from the begining (default) \n");
    printf("        -o='1(0)' output type: -o=1 - Save output data in file: 'Result.txt' \n");
    printf("                               -o=0 - Print output data on screen \n");
    printf("        -s='SEPARATOR' - default separator: '\\n' \n");
    printf("        -h - help \n");
}

/* Function to safe free all input parameters */
void safe_exit(input_t *pInputData)
{
    if (pInputData->szFilePath != NULL)
    {
        free(pInputData->szFilePath);
    }
    if (pInputData->szMask != NULL)
    {
        free(pInputData->szMask);
    }
    if (pInputData->szSeparator!= NULL && (*pInputData->szSeparator) != '\n' )
    {
        free(pInputData->szSeparator);
    }
}

/* Main programm function */
int main(int argc, const char * argv[])
{
    int i; input_t pInputData = {NULL,NULL,NULL,0,0,0,0};
    
    /* input parametters checking */
    if (argc==1)
    {
        file_create(8500000);
        return SUCCESS;
    }
    else
    {
        for (i = 1; i<argc; i++)
        {
            switch (start_parameters_parsing(argv[i],&pInputData))
            {
                case HELP: print_help(); safe_exit(&pInputData); return SUCCESS;
                case SUCCESS: break;
                default: safe_exit(&pInputData); return ERROR;
            }
        }
    }
    if ( (pInputData.szMask==NULL) || (pInputData.szFilePath==NULL) )
    {
        /* Print error: not enaugh input parameters */
        printf("Input parammeters error: no file_path parammeters or mask parammeters \n");
        safe_exit(&pInputData);
        return ERROR;
    }
    
    default_setup(&pInputData);
    
    /* Structure for threads*/
    common_data_t* pstThreadData = threads_data_init(&pInputData);

    /* Initialisation of mutexes and semaphores*/
    if ( threads_sync_init() != SUCCESS) goto END_OF_PROGRAMM;
    
    /* Getting the start time */
    double work_time; time_t start_time, end_time; start_time = time(NULL);
    
    /* Start threads */
    if ( threads_start(pstThreadData) != SUCCESS) goto END_OF_PROGRAMM;
    
    /* wait until all threads are stopped */
    threads_join();
    
    /* Get and print the time of work*/
    end_time = time(NULL); work_time = difftime(end_time, start_time);
    printf("The program has finished after %.0f seconds \n", work_time);
    
END_OF_PROGRAMM:
    /* Destroying mutexes  and semaphores*/
    threads_sync_destroy();
    /* Destroying structures */
    list_destroy(pstThreadData->plWrightQueue);
    list_destroy(pstThreadData->plSearchQueue);
    free(pstThreadData);
    /* Destroy input parammeters */
    safe_exit(&pInputData);
    return SUCCESS;
}
