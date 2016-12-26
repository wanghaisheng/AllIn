#ifndef QTDEMO_H
#define QTDEMO_H

#include <windows.h>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QTimer>
#include <QThread>
#include <boost/thread/thread.hpp>
#include <boost/signals2/signal.hpp>
#include <base/base_pub.h>
#include "ui_qtdemo.h"
#include "common.h"
#include "message.h"

class Event;

class QtDemo : public QMainWindow
{
private:
    Q_OBJECT

signals:
    void explains();
    void ConnectStatus(const char* path, unsigned int msg);
    void UpdateProgress(int progress, int total);

 private slots:
    void HandleConnect(const char* path, unsigned int msg);

private:
    static boost::mutex mtx;
    static int connected;
    static int _stdcall ConnectCallBack(const char* path, unsigned int msg);
    static int _stdcall DevMsgCallBack(unsigned int uMsg, unsigned int wParam, long lParam,
                            unsigned char* data, unsigned char len);
    void ShowTimeElapsed(unsigned long begin);

    int register_conn_cb_;
    int register_msg_cb_;


public:
    QtDemo(QWidget *parent = 0);

    bool Init();

    void PushMessage(Message* msg);

    ~QtDemo();

    /////////////////////////////// 主界面 ////////////////////////////////

    //使用如“connect(ui.combo_bound_mac_, SIGNAL(activated(int)), this, SLOT(HandleBoundMac(int)));”
    //绑定信号-槽需要在头文件中使用private slots:声明槽响应函数
private slots:
    //tab切换
    void HandleTabChange(int index);

private:
    //打开设备
    void HandleBtnOpenDev();

    //关闭设备
    void HandleBtnCloseDev();

    void OpenDevPost(const Message* msg);
    void CloseDevPost(const Message* msg);

    /////////////////////////////// 设备控制页 ////////////////////////////

private:
    void InitDevControl();          //页面初始化
    
    void HandleReset();             //复位
    void HandleRestart();           //重启
    void HandleSystemInfo();        //获取系统信息
    void HandleExitEntainmainMode(); //退出维护模式
    void HandleEnterEntainmainMode();//进入维护模式
    void HandleFirewareVersion();    //固件版本号

    void HandleOpenSafeDoor();      //打开安全门
    void HandleCloseSafeDoor();     //关闭安全门
    void HandleDeviceStatus();      //设备状态
    void HandleDoorStatus();        //门状态

    void HandleSetSN();             //设置序列号
    void HandleGetSN();             //获取序列号

    void HandleSetPaperTimeout();   //进纸门超时提示时间
    void HandleOpenPaperDoor();     //弹出纸门

    void HandleOpenLED();           //打开补光灯
    void HandleCloseLED();          //关闭补光灯
private slots:
    void HandleSliderLuminance(int val); //灯亮度调节

private:
    void HandleBeepAlways();        //长鸣
    void HandleBeepInterval();      //间隔响
    void HandleDisableBeep();       //关闭蜂鸣器

    void HandleInfraredStatus();    //获取红外状态

    void HandleABCCheck();          //农行校准

    void HandleReadSavedMAC();       //读取设备保存的MAC

private slots:
    void HandleBoundMac(int index);  //已绑定MAC地址的combo

private:
    void HandleUnbinding();          //解绑
    void HandleReadLocalMac();       //获取本机MAC
    void HandleBinding();            //绑定MAC

    void HandleSetAuthCode();       //设置设备认证码
    void HandleGetAuthCode();       //获取设备认证码

    void HandleOpenSafeDoorAlarm();  //开启开门报警
    void HandleCloseSafeDoorAlarm(); //关闭开门报警
    void HandleOpenVibrationAlarm(); //开启振动报警
    void HandleCloseVibrationAlarm();//关闭振动报警

    void HandleEnterTestMode();     //进入测试模式
    void HandleExitTestMode();      //退出测试模式
    void HandleMoveX();             //移动X轴
    void HandleMoveY();             //移动Y轴
    void HandleTurnStamper();       //转章

    void HandleEnableDebug();       //开启debug
    void HandleDisableDebug();      //关闭debug

    void HandleStamperStatus();     //所有章状态

private:
    std::string GetLocalMAC();

private:
    static const int DEFAULT_LUMINANCE = 50; //默认补光灯亮度值

    struct MacSet {
        int combo_idx_;                         //成功解绑后删除combox条目索引号

        int unbind_idx_;                        //解绑mac地址(1或2)
        std::string unbind_mac_;                //解绑mac地址
        std::map<std::string, int> bound_mac_;  //绑定MAC, MAC地址--1/2

        MacSet() : combo_idx_(-1), unbind_idx_(-1) {}
    };

    MacSet macs_;

    bool test_mode_;        //测试模式标志

    /////////////////////////////// 拍照/盖章页 //////////////////////////////

private slots:
    void StampInk(int checked);

private:
    void HandleCapture();       //拍照
    void HandlePreStamp();      //选章

private:
    void InitSnapshot();

private:
    QImage* img_;

    /////////////////////////////// 升级页 ////////////////////////////////////

private:
    void InitUpdate();      //页面初始化

    void HandleBtnSelectUpdateFile();   //选择升级文件
    void HandleBtnStartUpdating();      //开始升级

public:
    int BinFileCheck(const unsigned char* pReadBinBuffer, int len);
    int Syscheck(unsigned char fileSys, int &sys);
    int MCUcheck_SendB1(unsigned char* pReadBinBuffer, int len);

private:
    void firewareUpdateThread();

private:
    std::string     bin_file_;
    boost::thread*  update_thread_;

    //设备重连事件
public:
    static HANDLE UpdateEvent;

    //////////////////////////////// EEPROM操作页 ////////////////////////////////////

private:
    void HandleBtnReadCapacityVersion();    //读大小及版本号
    void HandleWriteData();                 //写数据
    void HandleReadData();                  //读数据

    void HandleReadStampers();              //读取章状态
    void HandleLoadStamperMapping();        //加载章映射
    void HandleSaveStamperMapping();        //保存章映射

    void HandleWriteConvRatio();            //存储转换倍率
    void HandleReadConvRatio();             //读取转换倍率

    void HandleWriteKey();                  //写设备key值
    void HandleReadKey();                   //读设备key值

private:
    QString current_ids[6];                 //当前印章ID, “读取章状态”按钮会修改该值

    ///////////////////////////// RFID操作页 //////////////////////////////////

private:
    void InitRfidPage();            //页面初始化

    void HandleRfidTutorial();      //操作说明

    void HandleAllaStamperStatus(); //所有章状态
    void HandleRequestAllCard();    //请求所有卡

    void HandleSelectStamper();     //选章
    void HandleGetRFID();           //卡请求

private slots:
    void HandleKeyType(int index);

    //rfid页6个radiobutton
    void RfidStamper1();
    void RfidStamper2();
    void RfidStamper3();
    void RfidStamper4();
    void RfidStamper5();
    void RfidStamper6();

private:
    void HandleBtnSetBlockAddr();   //设置卡绝对地址
    void HandleVerifyKey();         //卡密码校验
    void HandleRFIDFactoryCode();   //RFID出厂密码
    void HandleWriteBlock();        //写块
    void HandleReadBlock();         //读块

private:
    struct RFIDSet{
        std::map<int, unsigned int> stamper_rfid;

        unsigned char stamper;
        unsigned int rfid;

        unsigned char key_type;
        unsigned char key[6];
        unsigned char block;
    };

    ///////////////////////////// 盖章操作 ///////////////////////////

    //盖章参数
    struct StampPara {
        static const int MAX_X = 275; //毫米
        static const int MAX_Y = 250; //毫米
        static const int MAX_ANGLE = 360; //度

        static const int DEFAULT_X = 100;
        static const int DEFAULT_Y = 100;
        static const int DEFAULT_ANGLE = 0;

        static const int DEFAULT_WAIT = 1;
        static const int DEFAULT_TIMES = 9999;

        unsigned int rfid;
        unsigned char stamp_idx; //印章号, 从1开始
        unsigned char ink;       //是否蘸印油, 1--是
        unsigned char wait;      //等待时间,秒
        unsigned short x;
        unsigned short y;
        unsigned short angle;

        StampPara() : ink(0), rfid(0) {}
    };

private:
    void InitStamp();               //页面初始化

    void HandleEnableFactory();     //开启工厂模式
    void HandleDisableFactory();    //关闭工厂模式

    void HandleStampReadStampers(); //读取章状态

private slots:
    void Stamper1();
    void Stamper2();
    void Stamper3();
    void Stamper4();
    void Stamper5();
    void Stamper6();

    void HandleCheckStampInk(int checked); //是否蘸印油
    void HandleSliderX(int val);
    void HandleSliderY(int val);
    void HandleSliderAngle(int val);

private:
    void HandleOridinary();         //普通盖章
    void HandleSelSeal();           //选章
    void HandleConfirmStamp();      //确认盖普通章
    void HandleCheckMark();         //骑缝章
    void HandldeCancelStamp();      //取消盖章(印章归位)

    void HandleReadStamp();         //读盖章信息

    void HandleReadABC();           //农行读取电子标签
    void HandleReadABCIndex();      //农行获取印章仓位号

private:
    unsigned int serial_;
    StampPara para_; //盖章参数

    ///////////////////////////// 其他接口 //////////////////////////////////

private:
    void InitOther();

private:
    void HandleLock();   //锁定印控仪
    void HandleUnlock(); //解锁印控仪
    void HandleIsLock(); //查看锁定状态

    void HandleWriteID(); //写印控仪编号
    void HandleReadID();  //读印控仪编号

    void HandleWriteBackupSN(); //写备用板序列号
    void HandleReadBackupSN();  //读备用板序列号

    void HandleSetSideDoor();   //设置侧门提示时间

    void HandleWriteMAC();      //写MAC
    void HandleReadMAC();       //读MAC

    void HandleReadVoltage();   //读电压

    void HandldeWriteCalPoints();   //写校准点
    void HandleReadPCalPoints();    //读校准点

    void HandleReadRange();         //获取物理范围

    void HandleReadAlarm();         //读报警器状态

    void HandleHardwareVer();       //读取硬件版本号

    ////////////////////////////// 定时器 ///////////////////////////////////

private slots:
    void TimerDone();

private:
    void StartTimer(uint16_t milliseconds);
    void HexStr2Decimal(std::string hex_str, unsigned char* decimal_str, unsigned int len);
    
    int RadioButtonSelected(QRadioButton** btns, int size);

    void EventWorker();

    bool IsOpened();

    void Info(const QString& msg)
    {
        QMessageBox::information(
            NULL,
            DIALOG_HEADER,
            msg,
            QMessageBox::Yes,
            QMessageBox::NoButton);
    }

private:
    Ui::QtDemoClass ui;
    bool open_called; //设备是否已被打开

    base::SynQueue<Message*> msg_queue_;
    base::SynQueue<Event*> event_queue_;
    boost::thread*  echo_thread_;

    RFIDSet rfid_;
    QTimer* timer_;
    QLabel* status_label_;
};

#endif // QTDEMO_H
