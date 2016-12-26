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
