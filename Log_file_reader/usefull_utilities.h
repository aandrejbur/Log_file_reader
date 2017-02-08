#ifndef usefull_utilities_h
#define usefull_utilities_h

#include <stdio.h>

/* Swoping the array */
void array_swap( char* array, int *counter );

/* Creating a big file */
void file_create(long iSize);

/* Print help information */
void print_help();

/* Safe move for multibyte symbols */
char *safe_move(char* szString, int imode, int iLength);

/* Copy string src to buffer dst of size dsize. */
size_t strlcpy(char *dst, const char *src, size_t dsize);

#endif /* usefull_utilities_h */
