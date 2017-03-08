// test_dumper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "mini_dump.h"

int main()
{
    InstallCrashReport();

    char str[] = "1234,454,35";
    char* pch;
    printf("Splitting string \"%s\" into tokens:\n", str);
    pch = strtok(str, ",");
    while (pch != NULL) {
        printf("%s\n", pch);
        pch = strtok(NULL, ",");
    }
    
    char *p = NULL;
    *p = 'b';
}
