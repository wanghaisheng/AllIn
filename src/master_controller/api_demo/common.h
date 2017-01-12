#ifndef QTDEMO_COMMON_H_
#define QTDEMO_COMMON_H_

#include <string>
#include <QString>

const QString DIALOG_HEADER = QString::fromLocal8Bit("封闭式印控机V2.4-主控测试程序");
const std::string kLogger = "QtDemo";
//#define INFINITE 0xffffffff
#define STATUS_TEXT 2000

enum OperationCMD {
    OC_OPEN_DEV_SUC = 0,
    OC_DEV_ALREADY_OPENED,
    OC_OPEN_DEV_FAIL,
    OC_CLOSE_DEV_SUC,
    OC_CLOSE_DEV_FAIL,
    OC_READ_CAPA_VERSION_FAIL,
    OC_DEV_NOT_OPENED,
    OC_WRITE_DATA_SUC,
    OC_WRITE_DATA_FAIL,
    OC_READ_DATA_SUC,
    OC_READ_DATA_FAIL,
};

#define MAX_CMD OC_READ_DATA_FAIL + 1

static QString cmd_des[MAX_CMD] = 
{
    QString::fromLocal8Bit("设备打开成功"),
    QString::fromLocal8Bit("设备已打开"),
    QString::fromLocal8Bit("设备打开失败"),
    QString::fromLocal8Bit("设备关闭成功"),
    QString::fromLocal8Bit("设备关闭失败"),
    QString::fromLocal8Bit("读容量大小及版本号失败"),
    QString::fromLocal8Bit("设备未打开"),
    QString::fromLocal8Bit("写数据成功"),
    QString::fromLocal8Bit("写数据失败"),
    QString::fromLocal8Bit("读数据成功"),
    QString::fromLocal8Bit("读数据失败"),
};

#endif //QTDEMO_COMMON_H_
