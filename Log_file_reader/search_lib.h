#ifndef search_lib_h
#define search_lib_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FOUND 42
#define NOT_FOUND 0

typedef int (search_function)( char* szline, void *psSearch );

typedef struct search_t{
    int iMode, iLength;
    char *szMask;
    search_function *search;
}search_t;

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


int compare_symbol_to_common_symbols(char *Symbol);

#endif /* search_lib_h */
