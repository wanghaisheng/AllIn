#ifndef QTDEMO_COMMON_H_
#define QTDEMO_COMMON_H_

#include <string>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <QString>
#include <QColor>

const QString DIALOG_HEADER = QString::fromLocal8Bit("农行封闭式印控机 自检工具");

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

enum ErrorCode {
    EC_X0           = 0,
    EC_X1,
    EC_X_CENTER,
    EC_Y0,
    EC_Y1,
    EC_LOCATION,
    EC_SELECT_STAMPER,
    EC_Z,

    EC_STARTUP,

    EC_X_BUSY,
    EC_Y_BUSY,
    EC_Z_BUSY,

    EC_OUTSIZE_MEM,
    EC_DOOR_TIMEOUT,

    EC_MAX
};

static QString err_des[EC_MAX] =
{
    QString::fromLocal8Bit("X0红外传感器故障"),
    QString::fromLocal8Bit("X1红外传感器故障"),
    QString::fromLocal8Bit("X零位红外传感器故障"),
    QString::fromLocal8Bit("Y0红外传感器故障"),
    QString::fromLocal8Bit("Y1红外传感器故障"),
    QString::fromLocal8Bit("Y章前定位红外传感器故障"),
    QString::fromLocal8Bit("抓章检测红外传感器故障"),
    QString::fromLocal8Bit("Z的转章到位红外传感器故障"),

    QString::fromLocal8Bit("开机时检测章安全状态错误"),

    QString::fromLocal8Bit("X电机忙"),
    QString::fromLocal8Bit("Y电机忙"),
    QString::fromLocal8Bit("Z电机忙"),

    QString::fromLocal8Bit("外部存储器设备故障"),
    QString::fromLocal8Bit("门开启超时提示")
};

static QString err_resolver[EC_MAX] =
{
    QString::fromLocal8Bit(""),
    QString::fromLocal8Bit(""),
    QString::fromLocal8Bit(""),
    QString::fromLocal8Bit(""),
    QString::fromLocal8Bit(""),
    QString::fromLocal8Bit(""),
    QString::fromLocal8Bit(""),
    QString::fromLocal8Bit(""),

    QString::fromLocal8Bit("章槽内章个数发生变化，重新做章印射，或因章被盗或非法替换"),

    QString::fromLocal8Bit(""),
    QString::fromLocal8Bit(""),
    QString::fromLocal8Bit(""),

    QString::fromLocal8Bit("点击章自动印射重启无效后，系统重新启动无效请联系维修人员"),
    QString::fromLocal8Bit("请关闭安全门及推纸门")
};

#endif //QTDEMO_COMMON_H_
