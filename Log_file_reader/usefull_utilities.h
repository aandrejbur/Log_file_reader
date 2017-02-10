#ifndef usefull_utilities_h
#define usefull_utilities_h

#include <stdio.h>

#include "includes.h"

/* Swoping the array */
void array_swap( char* array, int *counter );

/* Creating a big file */
void file_create(long iSize);

/* Print help information */
void print_help();

/* Copy string src to buffer dst of size dsize. */
size_t strlcpy_udev(char *dst, const char *src, size_t dsize);

/* Console progress bar */
void load_bar(unsigned long lCurrent, unsigned long lAll, int iFrequancy, int iWide);

/* Realoc string */
char* realoc_string(char* szLine, int *iCurent_LineSize);

/* if iCounter > 20000 sleep for */
void counter_check(int iCounter, int iThreshold, int iSleepTime);

#endif /* usefull_utilities_h */
