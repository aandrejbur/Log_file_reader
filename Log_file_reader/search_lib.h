#ifndef search_lib_h
#define search_lib_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "includes.h"

/* Return flags */
#define FOUND 42
#define NOT_FOUND 0

#define FORWARD 0
#define BACKWARD 1

/* Pointer to search function */
typedef int (search_function)( char* szline, void *psSearch );

/* A structure for search */
typedef struct search_t
{
    /* Length of the mask */
    int iLength;
    /* Mask for search */
    char *szMask;
    /* A pointer to the search function */
    search_function *search;
} search_t;

/* Safe move for multibyte symbols */
char *safe_move(char* szString, int imode, int iLength);

/* Compare a symbol to the common symbols */
int compare_symbol_to_common_symbols(char *Symbol);

/* Search functions coresponding to mask */
/* *mask* */
int search_mode_0(char* szLine, void *psSearch);
/* *mask */
int search_mode_1(char* szLine, void *psSearch);
/* mask* */
int search_mode_2(char* szLine, void *psSearch);
/* ?mask? */
int search_mode_3(char* szLine, void *psSearch);
/* ?mask */
int search_mode_4(char* szLine, void *psSearch);
/* mask? */
int search_mode_5(char* szLine, void *psSearch);
/* *mask? */
int search_mode_6(char* szLine, void *psSearch);
/* ?mask* */
int search_mode_7(char* szLine, void *psSearch);
/* mask */
int search_mode_8(char* szLine, void *psSearch);

/* Create a structure for Search */
search_t* compile_search_expression(char* szMask);

/* Destroy search_t structure */
void search_destroy(search_t *psSearch);

#endif /* search_lib_h */
