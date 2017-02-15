#ifndef QTDEMO_H
#define QTDEMO_H

#include <windows.h>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QPainter>
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
    void OpenDevPost(const Message* msg);
    void CloseDevPost(const Message* msg);

    /////////////////////////////// 设备控制页 ////////////////////////////

private:
    void InitDevControl();          //页面初始化

    void HandleOpenSafeDoor();      //打开安全门
    void HandleCloseSafeDoor();     //关闭安全门
    void HandleReadSafe();          // 安全门状态
    void HandleReadPaper();         // 进纸门状态

private:
    void HandleBeepOn();        //长鸣
    void HandleBeepOff();       //关闭蜂鸣器

    void HandleQuerySlots();    //获取红外状态

    void HandleABCCheck();          //农行校准

    void HandleReadSavedMAC();       //读取设备保存的MAC

private:
    void HandleReadLocalMac();       //获取本机MAC
    void HandleBinding();            //绑定MAC
    void HandleUnbinding();          //解绑
    void HandleQueryMAC();          // 查询MAC

    void HandleOpenSafeDoorAlarm();  //开启开门报警
    void HandleCloseSafeDoorAlarm(); //关闭开门报警
    void HandleOpenVibrationAlarm(); //开启振动报警
    void HandleCloseVibrationAlarm();//关闭振动报警

    void HandleQuerySN();           // 获取设备编号
    void HandleSetSN();             // 设置设备编号

    void HandleCnnStatus();
    void HandleOpenCnn();
    void HandleCloseCnn();

    void HandleOpenPaper();

    void HandleOpenPaperLED();
    void HandleClosePaperLED();
    void HandleOpenSafeLED();
    void HandleCloseSafeLED();

    void HandleSetResolution();
    void HandleSetDPI();

    void HandleStartRecord();
    void HandleStopRecord();

    void HandleGetModelType();

private slots:
    void HandleErrCodeChange(const QString &);

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

protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    QPoint      mouse_press_pt_;
    QPoint      mouse_moving_pt_;

private slots:
    void HandleSelectImg(int index);    // 选择图片(原图或切图)
    void HandleCamListChange(int index);

private:
    void HandleOriImgClick();   // 点按原图
    void HandleCutImgClick();   // 点按切图
    void HandleCapture();       // 拍照
    void HandleSelectPic();     // 选择图片
    void HandlePreStamp();      // 选章

    void HandleIllusrate();
    void HandleCheckStamp();    // 校准盖章

    void HandleRecogCode();     // 版面、验证码识别
    void HandleRecogEle();      // 要素识别

    int which_cam_;
    void HandleOpenCam();
    void HandleCloseCam();
    void HandleQueryCam();

private:
    void InitSnapshot();

private:
    bool    capture_suc;    // 拍照并显示图片是否成功
    int     img_type_;      // 0-原图, 1-切图
    QImage* img_;

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

        StampPara() : ink(0), rfid(0), stamp_idx(0) {}
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

private:
    void HandleOridinary();         // 普通用印
    void HandleAutoStamp();         // 自动用印
    void HandlePrepare();           // 准备用印

    void HandleFinishStamp();       // 结束用印
    void HandleReleaseMachine();    // 释放印控机

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

    void HandleCheckParam();         //获取物理范围

    void HandleReadAlarm();         //读报警器状态

    void HandleReadRFID();

    ////////////////////////////// 定时器 ///////////////////////////////////

private slots:
    void TimerDone();

private:
    void StartTimer(uint16_t milliseconds);
    void HexStr2Decimal(std::string hex_str, unsigned char* decimal_str, unsigned int len);
    
    int RadioButtonSelected(QRadioButton** btns, int size);

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

    QTimer* timer_;
    QLabel* status_label_;
};

#endif // QTDEMO_H
