#include "usfull_utilities.h"


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
