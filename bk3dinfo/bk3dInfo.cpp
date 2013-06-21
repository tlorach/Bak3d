// bk3dInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define NOGZLIB
#include "bk3dEx.h"
#include "bk3dDbgFuncs.inl"

using namespace bk3d;

int main(int argc, char* argv[])
{
    bool bWait = true;
    if(argc <= 1)
    {
        fprintf(stderr, "%s <bk3d file>\n", argv[0]);
        return 1;
    }
    if(argc > 2)
    {
        if(!strcmp(argv[2], "nowait"))
            bWait = false;
    }
    FileHeader *pFile;
    pFile = load(argv[1]);
    if(!pFile)
    {
        fprintf(stderr, "failed to load %s\n", argv[1]);
		if(bWait)
		{
			printf("press 'enter' to end\n");
			getc(stdin);
		}
        return 1;
    }
    FileHeader_debugDumpAll(pFile, 0, NULL);
    if(bWait)
    {
        printf("press 'enter' to end\n");
        getc(stdin);
    }
	return 0;
}

