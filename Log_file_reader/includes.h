#ifndef includes_h
#define includes_h

#include "list_t.h"
#include "usefull_utilities.h"
#include "search_lib.h"
#include "threads.h"

/* Return flags */
#define HELP 911

typedef struct input_t
{
    /* Input parameters */
    char *szFilePath, *szMask, *szSeparator;
    int iScanTail, iMaxLines, iAmount, iOut;
} input_t;

#endif /* includes_h */
