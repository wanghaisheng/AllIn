// test_dumper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "dumper_api.h"


int _tmain(int argc, _TCHAR* argv[])
{
    InstallMiniDumper();

    char *p = NULL;
    *p = 'b';

	return 0;
}

