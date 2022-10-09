#include "stdafx.h"
#include "CommandLine.h"
#include <string.h>
#include <cstdlib>

CommandLine::CommandLine(int nArgsCount, char** args)
{
    if (!args)
    {
        char* tmp{};
        this->argc = 0;
        this->argv = &tmp;
    }
    else {
        this->argc = nArgsCount;
        this->argv = args;
    }
}

CommandLine::~CommandLine()
{
}

int CommandLine::FindParam(char* psz) const
{
    for (int i = 1; i < this->argc; ++i)
    {
        if (!_stricmp(this->argv[i], psz))
            return i;
    }
    return -1;
}

bool CommandLine::HasParam(char* psz) const
{
    return FindParam(psz) != -1;
}

bool CommandLine::HasParam(const char* psz) const
{
    return FindParam((char*)psz) != -1;
}

char* CommandLine::GetParamAtIdx(int idx) const
{
    // + 1 to skip the exe
    return this->argv[idx + 1];
}

unsigned int CommandLine::ArgC()
{
    return this->argc - 1;
}

char* CommandLine::GetParamValue(char* szArg, char* szDefault) const
{
    int idx = FindParam(szArg);
    if (idx == this->argc - 1 || idx == 0)
        return szDefault;

    char* szNextParam = this->argv[idx + 1];

    if (!szNextParam)
        return szDefault;

    if (szNextParam[0] == '-')
        return szDefault;

    return szNextParam;
}

const char* CommandLine::GetParamValue(const char* szArg, const char* szDefault) const
{
    int idx = FindParam((char*)szArg);
    if (idx == this->argc - 1 || idx <= 0)
        return szDefault;

    char* szNextParam = this->argv[idx + 1];

    if (!szNextParam)
        return szDefault;

    if (szNextParam[0] == '-')
        return szDefault;

    return szNextParam;
}