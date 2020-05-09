#include <time.h>
#include <stdio.h>
#include <string>

#include "date.h"

//This returns the date as a c string
char *GetDate(int Format)
{
    char *Ret = new char[24];

    time_t Raw;
    time(&Raw);
    tm *Time = localtime(&Raw);

    switch(Format)
    {
        case FORMAT_YDM:
            {
                sprintf(Ret, "%04d-%02d-%02d_%02d-%02d-%02d", Time->tm_year + 1900, Time->tm_mday, Time->tm_mon + 1, Time->tm_hour, Time->tm_min, Time->tm_sec);
                break;
            }
        case FORMAT_YMD:
            {
                sprintf(Ret, "%04d-%02d-%02d_%02d-%02d-%02d", Time->tm_year + 1900, Time->tm_mon + 1, Time->tm_mday, Time->tm_hour, Time->tm_min, Time->tm_sec);
                break;
            }
    }

    return Ret;
}

//this returns the time for the top bar
std::string RetTime()
{
    time_t Raw;
    time(&Raw);
    tm *Time = localtime(&Raw);

    char Tmp[32];

    sprintf(Tmp, "%02d:%02d", Time->tm_hour, Time->tm_min);

    return std::string(Tmp);
}
