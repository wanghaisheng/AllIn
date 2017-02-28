#include <iostream>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <stdlib.h>

typedef int DWORD;
enum FontLib {
    image = 0,
    eng = 1
};

#include "../mc_dll/ImgProcAndReco.h"
#include "seria.h"

#pragma comment(lib, "ImageProcess.lib")

enum CmdType {
    CT_INIT_MACHINE = 1,    //初始化
    CT_BIND_MAC,            //绑定MAC
    CT_UNBIND_MAC,          //解绑MAC
};

void main()
{
    float x = 3.43;
    std::stringstream stream(std::stringstream::in | std::stringstream::out);;
    char x_str[64] = { 0 };
    stream << std::fixed << std::setprecision(2) << x;
    sprintf(x_str, "%s", stream.str());
    std::cout << stream.str() << std::endl;

    float x_copy = atof(stream.str().c_str());
    printf("x: %f\n", x_copy);
    return;

        
}
