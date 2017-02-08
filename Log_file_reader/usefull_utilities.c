#include "usefull_utilities.h"

#define FORWARD 0
#define BACKWARD 1

char *rus[] ={
    "Программирование - твоя главная страсть. И да не будет у тебя страсти главней.",
    "Не сотвори себе кумира из конкретной технологии. Ибо программирование требует постоянного развития, а технологии-кумиры останавливают развитие.",
    "Не возноси хвальбу программированию в неподходящей компании. Ты сам себя накажешь, ибо будешь не понят, и люди отвернутся от тебя.",
    "Работай много и хорошо, но не забывай и про отдых. Ибо нет ничего страшнее, чем код усталого, засыпающего программиста.",
    "Уважай учителей и учеников своих. Постоянно учись и учи окружающих, чтобы было тебе всё легче и легче делать всё более и более сложные вещи.",
    "Не убий в себе ребенка. Не забывай эмоции от первого запуска первой написанной тобой программы и воспринимай каждую следующую, как ту - первую.",
    "Не изменяй программированию. Ибо программист может стать кем угодно, но этот кто угодно обратно программистом уже не станет.",
    "Не кради код ближнего своего.",
    "Не программируй то, что может принести вред другим. Ибо встав раз на путь дьявола - на нем и останешься.",
    "Не завидуй ближнему твоему, если он умеет лучше программировать. Ибо программирование - это божественный дар, но его можно развить. Так что не завидуй, а развивай.",
};

char *eng[] = {
    "Thou shalt run lint frequently and study its pronouncements with care, for verily its perception and judgement oft exceed thine.",
    "Thou shalt not follow the NULL pointer, for chaos and madness await thee at its end.",
    "Thou shalt cast all function arguments to the expected type if they are not of that type already, even when thou art convinced that this is unnecessary, lest they take cruel vengeance upon thee when thou least expect it.",
    "If thy header files fail to declare the return types of thy library functions, thou shalt declare them thyself with the most meticulous care, lest grievous harm befall thy program.",
    "Thou shalt check the array bounds of all strings (indeed, all arrays), for surely where thou typest ``foo'' someone someday shall type ``supercalifragilisticexpialidocious''.",
    "If a function be advertised to return an error code in the event of difficulties, thou shalt check for that code, yea, even though the checks triple the size of thy code and produce aches in thy typing fingers, for if thou thinkest ``it cannot happen to me'', the gods shall surely punish thee for thy arrogance.",
    "Thou shalt study thy libraries and strive not to reinvent them without cause, that thy code may be short and readable and thy days pleasant and productive.",
    "Thou shalt make thy program's purpose and structure clear to thy fellow man by using the One True Brace Style, even if thou likest it not, for thy creativity is better used in solving problems than in creating beautiful new impediments to understanding.",
    "Thy external identifiers shall be unique in the first six characters, though this harsh discipline be irksome and the years of its necessity stretch before thee seemingly without end, lest thou tear thy hair out and go mad on that fateful day when thou desirest to make thy program run on an old system.",
    "Thou shalt foreswear, renounce, and abjure the vile heresy which claimeth that ``All the world's a VAX'', and have no commerce with the benighted heathens who cling to this barbarous belief, that the days of thy program may be long even though the days of thy current machine be short.",
};

/* Printing a big true file */
void file_create(long iSize){
    FILE *file = NULL;
    long lLines = 0;
    file = fopen("Programmer_Commandments.txt", "w+");
    char szLine[2024];
    int iLineNumber = 0;
    while (lLines < iSize)
    {
        /* RUS */
        for (iLineNumber = 0; iLineNumber<10; iLineNumber++)
        {
            sprintf(szLine, "Строка: %ld Заповедь %d: %s \n",lLines+1,iLineNumber+1, rus[iLineNumber]);
            fprintf(file, "%s", szLine);
            lLines++;
            
        }
        /* ENG */
        for (iLineNumber = 0; iLineNumber<10; iLineNumber++)
        {
            sprintf(szLine, "Line: %ld Commandment %d: %s \n",lLines+1,iLineNumber+1, eng[iLineNumber]);
            fprintf(file, "%s", szLine);
            lLines++;
        }
    }
    fclose(file);
}

/* Array swap function */
void array_swap( char* array, int *counter )
{
    char cTempC;
    int i;
    for (i = 0; i < *counter/2; i++)
    {
        cTempC = array[i];
        (array[i]) = (char)(array[*counter-1-i]);
        array[*counter-1-i]=cTempC;
    }
}

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
            while ((unsigned char)*(pNewPosition -1)>= 0xC0) {
                pNewPosition++;;
            }
            if ( (unsigned char)*pNewPosition >= 0x80 && (unsigned char)*pNewPosition <=0xC0)
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


/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t dsize)
{
    const char *osrc = src;
    size_t nleft = dsize;
    
    /* Copy as many bytes as will fit. */
    if (nleft != 0)
    {
        while (--nleft != 0)
        {
            if ((*dst++ = *src++) == '\0')
            {
                break;
            }
        }
    }
    /* Not enough room in dst, add NUL and traverse rest of src. */
    if (nleft == 0)
    {
        if (dsize != 0)
        {
            *dst = '\0';		/* NUL-terminate dst */
        }
        while (*src++)
            ;
    }
    return(src - osrc - 1);	/* count does not include NUL */
}

/* Print help information */
void print_help()
{
    printf("Programm call syntax: ./log_reader -f='FILE_PATH' -m='MASK' -c='MAX LINES'(default 10000) \n");
    printf("                      -d='1(0)'File scan direction: -d=1 - from tail\n");
    printf("                                                    -d=2 - from the begining (default) \n");
    printf("                      -o='1(0)' output type: -o=1 - Save output data in file: 'Result.txt' \n");
    printf("                                             -o=0 - Print output data on screen \n");
    printf("                      -s='SEPARATOR' default separator: '\\n' \n");
    printf("                      -h - help \n");
}
