#include <iostream>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <stdlib.h>

class FuncTest {

    typedef void(FuncTest::*FuncPtr)(int a);

public:
    FuncTest() {
        ptr_vec_.push_back(&FuncTest::test1);
        ptr_vec_.push_back(&FuncTest::test2);
        ptr_vec_.push_back(&FuncTest::test3);
    }

    void print(int which, int x) {
        FuncPtr ptr = ptr_vec_.at(which - 1);
        (this->*ptr)(x);
    }

private:
    void test1(int x) {
        std::cout << "test1: " << x << std::endl;
    }

    void test2(int x) {
        std::cout << "test2: " << x << std::endl;
    }

    void test3(int x) {
        std::cout << "test3: " << x << std::endl;
    }

private:
    std::vector<FuncPtr> ptr_vec_;
};


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
    FuncTest ft;
    ft.print(1, 3);
    ft.print(2, 4);
    ft.print(3, 5);
    return;


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
