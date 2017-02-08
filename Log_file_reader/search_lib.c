#include "search_lib.h"
#include "usefull_utilities.h"
/* Function to compare the symbol to the common symbols */
int compare_symbol_to_common_symbols(char *Symbol)
{
    /* Common symbols*/
    char szSymbols[37] = ",. -!\"#$%&*;<=>@[]^_`{|}()№'~@:+?/";
    int i;
    for (i=0; i<36; i++)
    {
        if (strncmp(Symbol,szSymbols + i,1) == 0)
        {
            return FOUND;
        }
    }
    return NOT_FOUND;
}

/* Search functions coresponding to mask */
/* '*mask*' */
int search_mode_0(char* szLine, void *psSearch)
{
    char *cPosition = NULL;
    search_t* psSearch_temp = (search_t*)psSearch;
    cPosition = strstr(szLine, psSearch_temp->szMask);
    if ( cPosition == NULL)
    {
        return NOT_FOUND;
    }
    else return FOUND;
}

/* '*mask' */
int search_mode_1(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine;
    
    while (cPosition != NULL)
    {
        if ((cPosition = strstr(cPosition, psSearch_temp->szMask))!=NULL )
        {
            cPosition= safe_move(cPosition, 0, psSearch_temp->iLength);
            if (cPosition =='\0')
            {
                return FOUND;
            }
            else if (compare_symbol_to_common_symbols(cPosition) == FOUND)
            {
                return FOUND;
            }
        }
        else return NOT_FOUND;
    }
    return NOT_FOUND;
}
/* 'mask*' */
int search_mode_2(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine, *pcPrevious = NULL;
    while (cPosition!=NULL)
    {
        if ((cPosition=strstr(cPosition, psSearch_temp->szMask))!=NULL)
        {
            /*Check the previous symbol*/
            if (cPosition == szLine)
            {
                return FOUND;
            }
            else
            {
                pcPrevious = safe_move(cPosition, 1, 1);
                if (pcPrevious == szLine)
                {
                    return  FOUND;
                }
                if (compare_symbol_to_common_symbols(pcPrevious) == FOUND)
                {
                    return FOUND;
                }
            }
            cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
        }
        else return NOT_FOUND;
    }
    return NOT_FOUND;
    
}

/* '?mask?' */
int search_mode_3(char* szLine, void *psSearch)
{
    
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine, *cNext = NULL, *cPrevius = NULL;
    
    while (cPosition!=NULL)
    {
        int iStatus = 0;
        if ((cPosition=strstr(cPosition, psSearch_temp->szMask))!=NULL)
        {
            /*Check the previous symbol*/
            if (cPosition != szLine)
            {
                cPrevius = safe_move(cPosition, 1,1);
                if (cPrevius==szLine)
                {
                    iStatus++;
                }
                else
                {
                    cPrevius = safe_move(cPrevius, 1, 1);
                    if (cPrevius==szLine)
                    {
                        iStatus++;
                    }
                    else if(compare_symbol_to_common_symbols(cPrevius)== FOUND)
                    {
                        iStatus++;
                    }
                    else goto IF_END;
                }
            }
            /*check the Next symbol*/
            cNext = safe_move(cPosition, 0, psSearch_temp->iLength);
            if (*cNext != '\0')
            {
                cNext = safe_move(cNext, 0, 1);
                if (*cNext == '\0')
                {
                    iStatus++;
                }
                else if (compare_symbol_to_common_symbols(cNext) == FOUND)
                {
                    iStatus++;
                }
                else goto IF_END;
            }
            
            if (iStatus==2)
            {
                return FOUND;
            }
            IF_END:
            cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
            cPosition = safe_move(cPosition,0, psSearch_temp->iLength);
        }
        else return NOT_FOUND;
    }
    return NOT_FOUND;
}

/* '?mask' */
int search_mode_4(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine, *cNext, *cPrevius;
    int iStatus = 0;
    
    while (cPosition!=NULL)
    {
        iStatus = 0;
        if ((cPosition=strstr(cPosition, psSearch_temp->szMask))!=NULL)
        {
            cNext = safe_move(cPosition, 0, psSearch_temp->iLength);
            cPrevius = safe_move(cPosition, 1, 1);
            /*Check the previous symbol*/
            if (cPosition != szLine)
            {
                if (cPrevius==szLine)
                {
                    iStatus++;
                }
                else
                {
                    cPrevius = safe_move(cPrevius, 1, 1);
                    if (cPrevius==szLine)
                    {
                        iStatus++;
                    }
                    else if(compare_symbol_to_common_symbols(cPrevius)== FOUND)
                    {
                        iStatus++;
                    }
                    else goto IF_END;
                }
            }
            
            /* Check the next symbol */
            if (*cNext =='\0')
            {
                iStatus++;
            }
            else if (compare_symbol_to_common_symbols(cNext) == FOUND)
            {
                iStatus++;
            }else
            {
                goto IF_END;
            }
            
            /* Check, if both conditions are met */
            if (iStatus==2)
            {
                return FOUND;
            }
        
        IF_END:
            cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
        
        }
        else return NOT_FOUND;
    }
    return NOT_FOUND;
}
/* 'mask?' */
int search_mode_5(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine, *cNext = NULL, *cPrevius;
    int iStatus = 0;

    while (cPosition!=NULL)
    {
        iStatus = 0;
        
        if ((cPosition=strstr(cPosition, psSearch_temp->szMask))!=NULL)
        {
            /*Check the previous symbol*/
            if (cPosition == szLine)
            {
                iStatus++;
            }
            else
            {
                cPrevius = safe_move(cPosition, 1, 1);
                if (compare_symbol_to_common_symbols(cPrevius) == FOUND)
                {
                    iStatus++;
                }
                else goto IF_END;
            }
            
            /*check the Next symbol*/
            cNext = safe_move(cPosition, 0, psSearch_temp->iLength);
            if (*cNext != '\0')
            {
                cNext = safe_move(cNext, 0, 1);
                if (*cNext == '\0')
                {
                    iStatus++;
                }
                else if (compare_symbol_to_common_symbols(cNext) == FOUND)
                {
                    iStatus++;
                }
                else goto IF_END;
            }
            else goto IF_END;
            
            if (iStatus==2)
            {
                return FOUND;
            }
        IF_END:
            cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
        }
        else return NOT_FOUND;
    }
    return NOT_FOUND;
}
/* '*mask?' */
int search_mode_6(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine, *cNext = NULL;
    
    while (cPosition!=NULL)
    {
        if ((cPosition=strstr(cPosition, psSearch_temp->szMask))!=NULL)
        {
            cNext = safe_move(cPosition, 0, psSearch_temp->iLength);
            /*Check the next symbol*/
            /* If there is no character where ? must be */
            if (*cNext == '\0')
            {
                goto IF_END;
            }
            else
            {
                cNext = safe_move(cNext, 0, 1);
                if (*cNext == '\0')
                {
                    return FOUND;
                }
                else if (compare_symbol_to_common_symbols(cNext))
                {
                    return FOUND;
                }
                else goto IF_END;
            }
        }
        else return NOT_FOUND;
    IF_END:
        cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
    }
    return NOT_FOUND;
}
/* '?mask*' */
int search_mode_7(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine, *cPrevius = NULL;
    
    while (cPosition!=NULL)
    {
        if ((cPosition=strstr(cPosition, psSearch_temp->szMask))!=NULL)
        {
            cPrevius = safe_move(cPosition, 1, 1);
            /*Check the previous symbol*/
            if (cPosition != szLine)
            {
                if (cPrevius==szLine)
                {
                    return FOUND;
                }
                else
                {
                    cPrevius = safe_move(cPrevius, 1, 1);
                    if (cPrevius==szLine)
                    {
                        return FOUND;
                    }
                    else if(compare_symbol_to_common_symbols(cPrevius)== FOUND)
                    {
                        return FOUND;
                    }
                    else goto IF_END;
                }
            }
        IF_END:
            cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
        }else return NOT_FOUND;
    }
    return NOT_FOUND;
}

/* 'mask' */
int search_mode_8(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine, *cNext=NULL, *cPrevius=NULL;
    int iStatus = 0;
    
    
    while (cPosition!= NULL)
    {
        iStatus = 0;
        if ((cPosition=strstr(cPosition, psSearch_temp->szMask))!=NULL)
        {
            /*Check the previous symbol*/
            if (cPosition == szLine)
            {
                iStatus++;
            }
            else
            {
                cPrevius = safe_move(cPosition, 1, 1);
                if (compare_symbol_to_common_symbols(cPrevius) == FOUND)
                {
                    iStatus++;
                }
                else goto IF_END;
            }
            
            
            /* Check the next symbol */
            cNext = safe_move(cPosition, 0, psSearch_temp->iLength);
            if (*cNext =='\0')
            {
                iStatus++;
            }
            else if (compare_symbol_to_common_symbols(cNext) == FOUND)
            {
                iStatus++;
            }else
            {
                goto IF_END;
            }
            
            /* Check, if both conditions are met */
            if (iStatus==2)
            {
                return FOUND;
            }
            
            IF_END:
            cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
        } else return NOT_FOUND;
    }
    return NOT_FOUND;
}

/* Create a structure for Search */
search_t* compile_search_expression(char* szMask)
{
    if ( !szMask || szMask==NULL )
    {
        return NULL;
    }
    
    /* Create new Search_t element */
    search_t *psSearch_temp = malloc(sizeof(search_t));
    
    /* Get the length of szMask */
    int iLength = (int)strlen(szMask);
    psSearch_temp->szMask = NULL;
    
    /* Generate searcht_t corresponding to mask: '*mask*' */
    if (szMask[0]=='*'&&szMask[iLength-1]=='*')
    {
        psSearch_temp->search = &search_mode_0;
        szMask[iLength-1] = 0;
        psSearch_temp->szMask = malloc(iLength+2);
        strlcpy(psSearch_temp->szMask, szMask+1, iLength+2);
        psSearch_temp->iLength = iLength-2;
    }
    /* Generate searcht_t corresponding to mask: '?mask?' */
    else if (szMask[0]=='?'&&szMask[iLength-1]=='?')
    {
        psSearch_temp->search = &search_mode_3;
        psSearch_temp->szMask = malloc(iLength+2);
        szMask[iLength-1] = 0;
        
        strlcpy(psSearch_temp->szMask, szMask+1, iLength+2);
        psSearch_temp->iLength = iLength-2;
    }
    /* Generate searcht_t corresponding to mask: '*mask?' */
    else if (szMask[0]=='*'&&szMask[iLength-1]=='?')
    {
        psSearch_temp->search = &search_mode_6;
        psSearch_temp->szMask = malloc(iLength+2);
        szMask[iLength-1] = 0;
        strlcpy(psSearch_temp->szMask, szMask+1, iLength+2);
        psSearch_temp->iLength = iLength-2;
    }
    /* Generate searcht_t corresponding to mask: '?mask*' */
    else if (szMask[0]=='?'&&szMask[iLength-1]=='*')
    {
        psSearch_temp->search = &search_mode_7;
        psSearch_temp->szMask = malloc(iLength+2);
        szMask[iLength-1] = 0;
        strlcpy(psSearch_temp->szMask, szMask+1, iLength+2);
        psSearch_temp->iLength = iLength-2;
    }
    /* Generate searcht_t corresponding to mask: '*mask' */
    else if (szMask[0]=='*')
    {
        psSearch_temp->search = &search_mode_1;
        psSearch_temp->szMask = malloc(iLength+1);
        strlcpy(psSearch_temp->szMask, szMask+1, iLength+1);
        psSearch_temp->iLength = iLength-1;
    }
    /* Generate searcht_t corresponding to mask: '?mask' */
    else if (szMask[0]=='?')
    {
        psSearch_temp->search = &search_mode_4;
        psSearch_temp->szMask = malloc(iLength+1);
        strlcpy(psSearch_temp->szMask, szMask+1, iLength+1);
        psSearch_temp->iLength = iLength-1;
    }
    /* Generate searcht_t corresponding to mask: 'mask?' */
    else if (szMask[iLength-1]=='?')
    {
        psSearch_temp->search = &search_mode_5;
        psSearch_temp->szMask = malloc(iLength+1);
        szMask[iLength-1] = 0;
        strlcpy(psSearch_temp->szMask, szMask, iLength+1);
        psSearch_temp->iLength = iLength-1;
    }
    /* Generate searcht_t corresponding to mask: 'mask*' */
    else if (szMask[iLength-1]=='*')
    {
        psSearch_temp->search = &search_mode_2;
        psSearch_temp->szMask = malloc(iLength+1);
        szMask[iLength-1] = 0;
        strlcpy(psSearch_temp->szMask, szMask, iLength+1);
        psSearch_temp->iLength = iLength-1;
    }
    /* Generate searcht_t corresponding to mask: 'mask' */
    else
    {
        psSearch_temp->search = &search_mode_8;
        psSearch_temp->szMask = malloc(iLength+2);
        strlcpy(psSearch_temp->szMask, szMask, iLength+1);
        psSearch_temp->iLength = iLength;
    }
    return psSearch_temp;
}

/* Destroy search_t structure */
void search_destroy(search_t *psSearch)
{
    free(psSearch->szMask);
    psSearch->szMask=NULL;
    psSearch->search = NULL;
    free(psSearch);
    psSearch=NULL;
}
