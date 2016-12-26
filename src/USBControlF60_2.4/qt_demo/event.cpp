#include "event.h"
#include "message.h"
#include "common.h"
#include "qtdemo.h"
#include "USBProtocol.h"
#include "USBControlF60.h"

int WriteImageCovRatio(float* ratio, unsigned char len)
{
    int ret = ::WriteImageConvRatio(&ratio[0], &ratio[1]);
    return ret;
}


int ReadImageCovRatio(float* ratio, unsigned char len)
{
    int ret = ::ReadImageConvRatio(&ratio[0], &ratio[1]);
    return 0;
}

void test()
{
    //存储器版本号及可用空间大小
    unsigned char version;
    unsigned short mem_size;
    int ret = GetStorageCapacity(version, mem_size);

    //保存数据到印章机存储器中
    unsigned char data[] = { 20, 30, 2, 7, 8 };
    ret = WriteIntoStamper(100, data, 5);

    unsigned char ret_len = 0;
    unsigned char recv_data[512] = { 0 };
    ret = ReadStamperMem(100, 5, recv_data, ret_len);

    //根据印章仓位号获取RFID号
    unsigned int rfid = 0;
    ret = GetStamperID(4, rfid);

    unsigned char stamper;
    ret = GetStamper(rfid, stamper);
    if (stamper != 4)
        std::cout << "error";

    ret = SetStampMap();

    //获取印章映射关系
    char mapped_info[512] = { 0 };
    ret = GetStampMap(mapped_info, 512);

    unsigned char mac_len = 6;
    unsigned char mac1[7] = { 0 };
    unsigned char mac2[7] = { 0 };
    ret = WriteMAC(mac1, mac2, mac_len, mac_len);

    memset(mac1, 0, sizeof(mac1));
    memset(mac2, 0, sizeof(mac2));
    ret = ReadMAC(mac1, mac2, mac_len);

    unsigned char key_len = 16;
    unsigned char key[] = "df125483131deabf";
    ret = WriteKey(key, key_len);

    memset(key, 0, sizeof(key));
    ret = ReadKey(key, key_len);

    ret = SelectStamper(3);

    ret = CheckStampers();

    ret = EnterAuthMode();

    ret = ExitAuthMode();

    ret = EnableFactoryMode(1);

    ret = EnableFactoryMode(0);

    //操作块: 卡选择，卡请求，设置地址，密码比较，卡读写
    stamper = 1;
    ret = SelectStamper(stamper); //卡选择

    ret = GetStamperID(stamper, rfid); //卡请求

    unsigned char block = 10;
    ret = OperateBlock(block); //卡设置绝对地址

    unsigned char key_value[] = { 255, 255, 255, 255, 255, 255 };
    ret = VerifyKey(0, key_value, 6); //卡密码校验

    if (0 == ret) { //卡读写
        char data[] = "123";
        ret = WriteBlock(block, data, 3);
        
        memset(data, 0, sizeof(data));
        ret = ReadBlock(block, data, 3);
        data[0] = 1;
    }

    float ratio[2] = { 3.15f, 4.53f };
    ret = WriteImageCovRatio(ratio, 2);

    ratio[0] = 0.0f;
    ratio[1] = 0.0f;
    ret = ReadImageCovRatio(ratio, 2);

    unsigned short points[] = {
        25, 32,
        10, 40,
        5, 30,
        39, 59,
        12, 31
    };
    ret = WriteCalibrationPoint(points, 10);

    memset(points, 0, sizeof(points));
    ret = CalibrationPoint(points, 10);

    float x = 3.5f;
    float y = 4.67f;
    ret = WriteImageConvRatio(&x, &y);

    x = 0.0f;
    y = 0.0f;
    ret = ReadImageConvRatio(&x, &y);
    
    unsigned int rfids[7] = { 0 };
    unsigned char stampers = 0;
    ret = ReadAllRFID(rfids, 7, &stampers);

    ;
}

void OpenEventS::Execute()
{
    Message* msg = new (std::nothrow) Message(CMD_OPEN_DEV);
    int err = msg->err_ = ::FOpenDev(NULL);
    //test();
    ((QtDemo*)win_)->PushMessage(msg);
    delete this;
}

void CloseEvent::Execute()
{
    Message* msg = new (std::nothrow) Message(CMD_CLOSE_DEV);
    int err = msg->err_ =  ::FCloseDev();
    ((QtDemo*)win_)->PushMessage(msg);
    delete this;
}
