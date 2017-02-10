#include "search_lib.h"

/* Safe move for multibyte symbols */
char *safe_move(char* szString, int imode, int iLength)
{
    if (szString == NULL)
    {
        return NULL;
    }
    char *pNewPosition = NULL;
    switch (imode) {
            /*safe move forward*/
        case FORWARD:
            pNewPosition = szString+iLength;
            while ( (unsigned char)*pNewPosition >= 0x80 && (unsigned char)*pNewPosition <0xC0)
            {
                pNewPosition++;
            }
            break;
            /*safe move backward*/
        case BACKWARD:
            pNewPosition = szString-iLength;
            while (0x80 <= (unsigned char)pNewPosition[0] && (unsigned char)pNewPosition[0]<0xC0 )
            {
                pNewPosition--;
            }
            break;
        default:
            return NULL;
    }
    return pNewPosition;
}

/* Function to compare the symbol to the common symbols by the codes*/
int compare_symbol_to_common_symbols(char *Symbol)
{
    switch ((unsigned char)*Symbol)
    {
        case (0x20): case (0x21): case (0x22):
        case (0x23): case (0x24): case (0x25):
        case (0x26): case (0x27): case (0x28):
        case (0x29): case (0x2a): case (0x2b):
        case (0x2c): case (0x2d): case (0x2e):
        case (0x2f): case (0x3a): case (0x3b):
        case (0x3c): case (0x3d): case (0x3e):
        case (0x3f): case (0x40): case (0x5b):
        case (0x5c): case (0x5d): case (0x5e):
        case (0x5f): case (0x60): case (0x7b):
        case (0x7c): case (0x7d): case (0x7e):
        case (0x7f):
            return FOUND;
        default:
            return NOT_FOUND;
    }
}

/* Search functions coresponding to mask */
/* '*mask*' */
int search_mode_0(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    if ((strstr(szLine, psSearch_temp->szMask))==NULL)
    {
        return NOT_FOUND;
    }
    return FOUND;
}

/* '*mask' */
int search_mode_1(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine;
    
    while (cPosition != NULL)
    {
        if ((cPosition=strstr(cPosition, psSearch_temp->szMask))!=NULL)
        {
            /* Check next symbol */
            if (*(cPosition = safe_move(cPosition, 0, psSearch_temp->iLength)) == '\0')
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
                if ( (pcPrevious = safe_move(cPosition, 1, 1)) == szLine)
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
    int iStatus;
    
    while (cPosition!=NULL)
    {
        iStatus =0;
        if ((cPosition=strstr(cPosition, psSearch_temp->szMask))!=NULL)
        {
            /*Check the previous symbol*/
            if (cPosition != szLine)
            {
                if ( (cPrevius = safe_move(cPosition, 1,1) )==szLine)
                {
                    iStatus++;
                }
                else
                {
                    if ((cPrevius = safe_move(cPrevius, 1, 1))==szLine)
                    {
                        iStatus++;
                    }
                    else if(compare_symbol_to_common_symbols(cPrevius) != FOUND)
                    {
                        goto IF_END;
                    }
                    else iStatus++;
                }
            }
            /*check the Next symbol*/
            if (*(cNext = safe_move(cPosition, 0, psSearch_temp->iLength)) != '\0')
            {
                if (*(cNext = safe_move(cNext, 0,1)) == '\0')
                {
                    iStatus++;
                }
                else if (compare_symbol_to_common_symbols(cNext) != FOUND)
                {
                    goto IF_END;
                }
                else iStatus++;
            }
            
            if (iStatus==2) return FOUND;
            
            IF_END:
            cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
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
            else if (compare_symbol_to_common_symbols(cNext) != FOUND)
            {
                goto IF_END;
            }
            else iStatus++;
            
            /* Check, if both conditions are met */
            if (iStatus==2) return FOUND;
            
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
    char *cPosition = szLine, *cNext = NULL;
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
                if (compare_symbol_to_common_symbols(safe_move(cPosition, 1, 1)) != FOUND)
                {
                    goto IF_END;
                }
                else iStatus++;
            }
            
            /*check the Next symbol*/
            if (*(cNext = safe_move(cPosition, 0, psSearch_temp->iLength))!= '\0')
            {
                ;
                if (*(cNext = safe_move(cNext, 0, 1)) == '\0')
                {
                    iStatus++;
                }
                else if (compare_symbol_to_common_symbols(cNext) != FOUND)
                {
                    goto IF_END;
                }
                else iStatus++;
            }
            else goto IF_END;
            
            if (iStatus==2) return FOUND;
            
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
            ;
            /*Check the next symbol*/
            /* If there is no character where ? must be */
            if (*(cNext = safe_move(cPosition, 0, psSearch_temp->iLength)) == '\0')
            {
                goto IF_END;
            }
            else
            {
                if (*(cNext = safe_move(cNext, 0, 1)) == '\0')
                {
                    return FOUND;
                }
                else if (compare_symbol_to_common_symbols(cNext)!=FOUND)
                {
                    return FOUND;
                }
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
            /*Check the previous symbol*/
            if (cPosition != szLine)
            {
                if (( cPrevius = safe_move(cPosition, 1, 1))==szLine)
                {
                    return FOUND;
                }
                else
                {
                    if ((cPrevius = safe_move(cPrevius, 1, 1))==szLine)
                    {
                        return FOUND;
                    }
                    else if(compare_symbol_to_common_symbols(cPrevius) == FOUND)
                    {
                        return FOUND;
                    }
                }
            }
            cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
        }else return NOT_FOUND;
    }
    return NOT_FOUND;
}

/* 'mask' */
int search_mode_8(char* szLine, void *psSearch)
{
    search_t* psSearch_temp = (search_t*)psSearch;
    char *cPosition = szLine, *cNext=NULL;
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
                if (compare_symbol_to_common_symbols(safe_move(cPosition, 1, 1)) != FOUND)
                {
                    goto IF_END;
                }
                else iStatus++;
            }
            /* Check the next symbol */
            if (*(cNext = safe_move(cPosition, 0, psSearch_temp->iLength)) =='\0')
            {
                iStatus++;
            }
            else if (compare_symbol_to_common_symbols(cNext) != FOUND)
            {
                goto IF_END;
            }
            else iStatus++;
            
            /* Check, if both conditions are met */
            if (iStatus==2) return FOUND;
            
            IF_END:
            cPosition = safe_move(cPosition, 0, psSearch_temp->iLength);
        }
        else return NOT_FOUND;
    }
    return NOT_FOUND;
}

/* Create a structure for Search */
search_t* compile_search_expression(char* szMask)
{
    if ( !szMask || szMask==NULL ) return NULL;
    
    /* Create new Search_t element */
    search_t *psSearch_temp = malloc(sizeof(search_t));
    
    /* Get the length of szMask */
    int iLength = (int)strlen(szMask);
    
    /* allocate the memory for szmask */
    psSearch_temp->szMask = NULL;
    psSearch_temp->szMask = malloc(iLength+2);
    
    /* Generate searcht_t corresponding to mask: '*mask*' */
    if (szMask[0]=='*'&&szMask[iLength-1]=='*')
    {
        psSearch_temp->search = &search_mode_0;
        szMask[iLength-1] = 0;
        strlcpy_udev(psSearch_temp->szMask, szMask+1, iLength+1);
        psSearch_temp->iLength = iLength-2;
    }
    /* Generate searcht_t corresponding to mask: '?mask?' */
    else if (szMask[0]=='?'&& szMask[iLength-1]=='?')
    {
        psSearch_temp->search = &search_mode_3;
        szMask[iLength-1] = 0;
        strlcpy_udev(psSearch_temp->szMask, szMask+1, iLength+1);
        psSearch_temp->iLength = iLength-2;
    }
    /* Generate searcht_t corresponding to mask: '*mask?' */
    else if (szMask[0]=='*'&&szMask[iLength-1]=='?')
    {
        psSearch_temp->search = &search_mode_6;
        szMask[iLength-1] = 0;
        strlcpy_udev(psSearch_temp->szMask, szMask+1, iLength+1);
        psSearch_temp->iLength = iLength-2;
    }
    /* Generate searcht_t corresponding to mask: '?mask*' */
    else if (szMask[0]=='?'&&szMask[iLength-1]=='*')
    {
        psSearch_temp->search = &search_mode_7;
        szMask[iLength-1] = 0;
        strlcpy_udev(psSearch_temp->szMask, szMask+1, iLength+1);
        psSearch_temp->iLength = iLength-2;
    }
    /* Generate searcht_t corresponding to mask: '*mask' */
    else if (szMask[0]=='*')
    {
        psSearch_temp->search = &search_mode_1;
        strlcpy_udev(psSearch_temp->szMask, szMask+1, iLength+1);
        psSearch_temp->iLength = iLength-1;
    }
    /* Generate searcht_t corresponding to mask: '?mask' */
    else if (szMask[0]=='?')
    {
        psSearch_temp->search = &search_mode_4;
        strlcpy_udev(psSearch_temp->szMask, szMask+1, iLength+1);
        psSearch_temp->iLength = iLength-1;
    }
    /* Generate searcht_t corresponding to mask: 'mask?' */
    else if (szMask[iLength-1]=='?')
    {
        psSearch_temp->search = &search_mode_5;
        szMask[iLength-1] = 0;
        strlcpy_udev(psSearch_temp->szMask, szMask, iLength+1);
        psSearch_temp->iLength = iLength-1;
    }
    /* Generate searcht_t corresponding to mask: 'mask*' */
    else if (szMask[iLength-1]=='*')
    {
        psSearch_temp->search = &search_mode_2;
        szMask[iLength-1] = 0;
        strlcpy_udev(psSearch_temp->szMask, szMask, iLength+1);
        psSearch_temp->iLength = iLength-1;
    }
    /* Generate searcht_t corresponding to mask: 'mask' */
    else
    {
        psSearch_temp->search = &search_mode_8;
        strlcpy_udev(psSearch_temp->szMask, szMask, iLength+1);
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
