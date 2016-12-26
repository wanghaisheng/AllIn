#include "boc_api.h"

//////////////////////////// 初始化印控机 /////////////////////////////////

class InitMachNT:public MC::NotifyResult {

public:
    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "初始化印控机, ec: " << ec << ", 认证码: " << data1.c_str() << std::endl;
    }
};

void boc_test()
{
    MC::BOCApi boc("中行");

    MC::NotifyResult* notify = new (std::nothrow) InitMachNT();
    boc.InitMachine("boc_stamper", notify);
}
