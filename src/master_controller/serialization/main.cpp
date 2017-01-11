#include <iostream>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>

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
    std::string cut_img = "C:\\pj\\bin\\w32d\\cut.img";
    std::string model_type = "123";
    char out_mo_type[256] = { 0 };
    char out_Vocher_Number[256] = { 0 };
    char out_trace_code[256] = { 0 };
    int x = 0;
    int y = 0;
    int angle = 0;
    int result = RecoModelTypeAndCodeAndAngleAndPointByImg(
        cut_img.c_str(),
        (char*)model_type.c_str(),
        out_mo_type,
        out_Vocher_Number,
        out_trace_code,
        x,
        y,
        angle);    printf("result: %d\n", result);


    char str1[] = { 2, 3, 125, 126, 127};
    char str2[] = {'2', '3'};
    std::cout << str1 << ", " << strlen(str1) << std::endl; 
    std::cout << str2 << ", " << strlen(str1) << std::endl;
    return;

    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::cout << uuid << std::endl;
    std::string uuid_str = boost::lexical_cast<std::string>(uuid);
    std::cout << "uuid_str: " << uuid_str << ", size: " << uuid_str.length() << std::endl;

    CmdType ct = CT_INIT_MACHINE;
    char name[] = "yongwei";
    int age = 30;
    
    XSpace xs;
    xs << ct << name;
    xs.Trim();

    xs.Untrim();
    CmdType ct_uns;
    char name_uns[8] = { 0 };
    int age_uns = 0;
    xs >> ct_uns >> name_uns;

    std::cout << ct_uns << ", " << name_uns << ", " << age_uns << std::endl;
}
