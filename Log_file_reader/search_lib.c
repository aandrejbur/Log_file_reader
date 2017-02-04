#include "search_lib.h"

/* Function to compare the symbol to the common symbols */
int compare_symbol_to_common_symbols(char *Symbol)
{
    int i;
    char szSymbols[36] = ",. -!\"#$%&*;<=>@[]^_`{|}()№'~@:+?/";
    for (i=0; i<36; i++)
    {
        if (strncmp(Symbol,szSymbols + i,1) == 0)
        {
            return FOUND;
        }
    }
    return NOT_FOUND;
}

/* Search funtions coresponding to mask */
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
    else
    {
        return FOUND;
    }
}

/* '*mask' */
int search_mode_1(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    
    char *cPosition = NULL;
    
    if ((cPosition=strstr(szLine, psSearch_temp->szMask))!=NULL)
    {
        cPosition+=psSearch_temp->iLength;
        if (cPosition =='\0')
        {
            return FOUND;
        }
        else return compare_symbol_to_common_symbols(cPosition);
    }
    else return NOT_FOUND;
}
/* 'mask*' */
int search_mode_2(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = NULL;
    
    if ((cPosition=strstr(szLine, psSearch_temp->szMask))!=NULL)
    {
        
        if (cPosition == szLine)
        {
            return FOUND;
        }
        cPosition--;
        return compare_symbol_to_common_symbols(cPosition);
    }
    else return NOT_FOUND;
}
/* '?mask?' */
int search_mode_3(char* szLine, void *psSearch)
{
    int iStatus = 0;
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = NULL, *cNext, *cPrevius;
    
    if ((cPosition=strstr(szLine, psSearch_temp->szMask))!=NULL)
    {
        cNext = cPosition+psSearch_temp->iLength;
        cPrevius = cPosition-1;
        if (cPosition == szLine)
        {
            return NOT_FOUND;
        }
        else
        {
            if (cPrevius==szLine)
            {
                return NOT_FOUND;
            }
            else if(compare_symbol_to_common_symbols(cPrevius)== FOUND)
            {
                return NOT_FOUND;
            }
            else
            {
                
                cPrevius--;
                if (cPrevius==szLine)
                {
                    iStatus++;
                }
                else if(compare_symbol_to_common_symbols(cPrevius)== FOUND)
                {
                    iStatus++;
                }
                else return NOT_FOUND;
            }
        }
        
        /*check the Next symbol*/
        if (cNext == '\0')
        {
            return NOT_FOUND;
        }
        cNext++;
        if (cNext == '\0')
        {
            iStatus++;
        }
        else if (compare_symbol_to_common_symbols(cNext) == FOUND)
        {
            iStatus++;
        }
        else return NOT_FOUND;
        
        if (iStatus==2)
        {
            return FOUND;
        }
        else return NOT_FOUND;
    }
    else return NOT_FOUND;
}
/* '?mask' */
int search_mode_4(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = NULL, *cNext, *cPrevius;
    int iStatus = 0;
    
    if ((cPosition=strstr(szLine, psSearch_temp->szMask))!=NULL)
    {
        cNext = cPosition+psSearch_temp->iLength;
        cPrevius = cPosition -1;
        /*Check the previous symbol*/
        if (cPosition == szLine)
        {
            return NOT_FOUND;
        }
        else
        {
            if (cPrevius==szLine)
            {
                iStatus++;
            }
            else
            {
                cPrevius--;
                if(compare_symbol_to_common_symbols(cPrevius))
                {
                    iStatus++;
                }
                else return NOT_FOUND;
            }
        }
        
        /* Check the next symbol */
        if (cNext == '\0')
        {
            iStatus++;
        }
        else if (compare_symbol_to_common_symbols(cNext) == FOUND)
        {
            iStatus++;
        }
        else return NOT_FOUND;
        
        if (iStatus==2)
        {
            return FOUND;
        }
        else return NOT_FOUND;
    }
    else return NOT_FOUND;
}
/* 'mask?' */
int search_mode_5(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = NULL, *cNext, *cPrevius;
    int iStatus = 0;
    
    if ((cPosition=strstr(szLine, psSearch_temp->szMask))!=NULL)
    {
        cNext = cPosition+psSearch_temp->iLength;
        cPrevius = cPosition -1;
        /*Check the previous symbol*/
        if (cPosition == szLine)
        {
            iStatus++;
        }
        else if (compare_symbol_to_common_symbols(cPrevius))
        {
            iStatus++;
        }
        else return NOT_FOUND;
        
        /*check the Next symbol*/
        if (cNext == '\0')
        {
            return NOT_FOUND;
        }
        cNext++;
        if (cNext == '\0')
        {
            iStatus++;
        }
        else if (compare_symbol_to_common_symbols(cNext) == FOUND)
        {
            iStatus++;
        }
        else return NOT_FOUND;
        
        if (iStatus==2)
        {
            return FOUND;
        }
        else
        {
            return iStatus;
        }
    }
    else return NOT_FOUND;
}
/* '*mask?' */
int search_mode_6(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = NULL;
    if ((cPosition=strstr(szLine, psSearch_temp->szMask))!=NULL)
    {
        cPosition +=psSearch_temp->iLength;
        /*Check the next symbol*/
        /* If there is no character where ? must be */
        if (cPosition == '\0')
        {
            return NOT_FOUND;
        }
        else
        {
            cPosition++;
            if (cPosition == '\0')
            {
                return FOUND;
            }
            else if (compare_symbol_to_common_symbols(cPosition))
            {
                return FOUND;
            }
            else return NOT_FOUND;
        }
    }
    else return NOT_FOUND;
}
/* '?mask*' */
int search_mode_7(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = NULL;
    if ((cPosition=strstr(szLine, psSearch_temp->szMask))!=NULL)
    {
        /*Check the previous symbol*/
        /* If there is no character where ? must be */
        if (cPosition == szLine)
        {
            return NOT_FOUND;
        }
        else
        {
            cPosition--;
            if (cPosition == szLine)
            {
                return FOUND;
            }
            else
            {
                cPosition--;
                if (compare_symbol_to_common_symbols(cPosition))
                {
                    return FOUND;
                }
                else return NOT_FOUND;
            }
        }
    }
    else return NOT_FOUND;
}

/* 'mask' */
int search_mode_8(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = NULL, *cNext, *cPrevius;
    int iStatus = 0;
    
    if ((cPosition=strstr(szLine, psSearch_temp->szMask))!=NULL)
    {
        cNext = cPosition+psSearch_temp->iLength;
        cPrevius = cPosition -1;
        /*Check the previous symbol*/
        if (cPosition == szLine)
        {
            iStatus++;
        }
        else if (compare_symbol_to_common_symbols(cPrevius))
        {
            iStatus++;
        }
        else return NOT_FOUND;

        
        /*check the Next symbol*/
        if (cNext == '\0')
        {
            iStatus++;
        }
        else if (compare_symbol_to_common_symbols(cNext) == FOUND)
        {
            iStatus++;
        }
        else return NOT_FOUND;
        
        
        if (iStatus==2)
        {
            return FOUND;
        }
        else
        {
            return iStatus;
        }
    }
    else return NOT_FOUND;
}

/* Create a structure for Search */
search_t* compile_search_expression(char* szMask)
{
    if (!szMask)
    {
        return NULL;
    }
    search_t *psSearch_temp = malloc(sizeof(search_t));
    /* Get the length of szmask */
    int iLength = (int)strlen(szMask);
    
    
    if (szMask[0]=='*'&&szMask[iLength-1]=='*')
    {
        psSearch_temp->search = &search_mode_0;
        psSearch_temp->szMask = malloc(iLength-1);
        memmove(psSearch_temp->szMask, szMask+1, iLength-2);
        psSearch_temp->szMask[iLength-1] = '\0';
        psSearch_temp->iLength = iLength-2;
        
        return psSearch_temp;
    }
    else if (szMask[0]=='?'&&szMask[iLength-1]=='?')
    {
        psSearch_temp->search = &search_mode_3;
        psSearch_temp->szMask = malloc(iLength-1);
        memmove(psSearch_temp->szMask, szMask+1, iLength-2);
        psSearch_temp->iLength = iLength-2;
        return psSearch_temp;
    }
    else if (szMask[0]=='*'&&szMask[iLength-1]=='?')
    {
        psSearch_temp->search = &search_mode_6;
        psSearch_temp->szMask = malloc(iLength-1);
        memmove(psSearch_temp->szMask, szMask+1, iLength-2);
        psSearch_temp->iLength = iLength-2;
        return psSearch_temp;
    }
    else if (szMask[0]=='?'&&szMask[iLength-1]=='*')
    {
        psSearch_temp->search = &search_mode_7;
        psSearch_temp->szMask = malloc(iLength-1);
        memmove(psSearch_temp->szMask, szMask+1, iLength-2);
        psSearch_temp->iLength = iLength-2;
        return psSearch_temp;
    }
    else if (szMask[0]=='*')
    {
        psSearch_temp->search = &search_mode_1;
        psSearch_temp->szMask = malloc(iLength);
        memmove(psSearch_temp->szMask, szMask+1, iLength-1);
        psSearch_temp->iLength = iLength-1;
        return psSearch_temp;
    }
    else if (szMask[0]=='?')
    {
        psSearch_temp->search = &search_mode_4;
        psSearch_temp->szMask = malloc(iLength);
        memmove(psSearch_temp->szMask, szMask+1, iLength-1);
        psSearch_temp->iLength = iLength-1;
        return psSearch_temp;
    }
    else if (szMask[iLength-1]=='?')
    {
        psSearch_temp->search = &search_mode_5;
        psSearch_temp->szMask = malloc(iLength);
        memmove(psSearch_temp->szMask, szMask, iLength-1);
        psSearch_temp->iLength = iLength-1;
        return psSearch_temp;
    }
    else if (szMask[iLength-1]=='*')
    {
        psSearch_temp->search = &search_mode_2;
        psSearch_temp->szMask = malloc(iLength);
        memmove(psSearch_temp->szMask, szMask, iLength-1);
        psSearch_temp->iLength = iLength-1;
        return psSearch_temp;
    }
    else
    {
        psSearch_temp->search = &search_mode_8;
        psSearch_temp->szMask = malloc(iLength+1);
        memmove(psSearch_temp->szMask, szMask, iLength);
        psSearch_temp->iLength = iLength;
        return psSearch_temp;
    }
    return NULL;
}
