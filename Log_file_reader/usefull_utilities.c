#include "usefull_utilities.h"

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
void file_create(int iSize){
    FILE *file = NULL;
    int lLines = 0;
    file = fopen("Programmer_Commandments.txt", "w+");
    char szLine[1024];
    int iLineNumber = 0;
    while (lLines < iSize)
    {
        /* RUS */
        for (iLineNumber = 0; iLineNumber<10; iLineNumber++)
        {
            sprintf(szLine, "Строка: %d Заповедь %d: %s \n",lLines+1,iLineNumber+1, rus[iLineNumber]);
            fprintf(file, "%s", szLine);
            lLines++;
        }
        /* ENG */
        for (iLineNumber = 0; iLineNumber<10; iLineNumber++)
        {
            sprintf(szLine, "Line: %d Commandment %d: %s \n",lLines+1,iLineNumber+1, eng[iLineNumber]);
            fprintf(file, "%s", szLine);
            lLines++;
        }
    }
    fclose(file);
}

/* Swoping the array */
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
