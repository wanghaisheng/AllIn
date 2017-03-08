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

    std::string circle1_str;
    std::string circle2_str;
    std::string circle3_str;
    std::string circle4_str;

    std::string buf_str;
    char buf[1024] = "327,235,120@2417,239,117@2419,1603,116@343,1609,122";

    buf_str = std::string(buf);
    std::string::size_type at_pos = buf_str.find_first_of("@");
    circle1_str = buf_str.substr(0, at_pos);

    buf_str = buf_str.substr(at_pos + 1, std::string::npos);
    at_pos = buf_str.find_first_of("@");
    circle2_str = buf_str.substr(0, at_pos);

    buf_str = buf_str.substr(at_pos + 1, std::string::npos);
    at_pos = buf_str.find_first_of("@");
    circle3_str = buf_str.substr(0, at_pos);

    buf_str = buf_str.substr(at_pos + 1, std::string::npos);
    circle4_str = buf_str;

    std::cout << circle1_str.c_str() << std::endl <<
        circle2_str.c_str() << std::endl <<
        circle3_str.c_str() << std::endl <<
        circle4_str.c_str() << std::endl;

    
    char *p = NULL;
    *p = 'b';
}
