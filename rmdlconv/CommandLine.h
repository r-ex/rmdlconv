#pragma once

class CommandLine
{
public:
    int argc = 0;
    char** argv;

    CommandLine(int nArgsCount, char** args);
    ~CommandLine();

    virtual int FindParam(char* psz) const;

    virtual bool HasParam(char* psz) const;
    virtual bool HasParam(const char* psz) const;

    virtual char* GetParamAtIdx(int idx) const;

    virtual unsigned int ArgC();

    virtual char* GetParamValue(char* pszArg, char* szDefault = (char*)"") const;
    virtual const char* GetParamValue(const char* pszArg, const char* szDefault = (char*)"") const;
};

