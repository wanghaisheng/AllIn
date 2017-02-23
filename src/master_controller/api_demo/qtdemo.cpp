#include "qtdemo.h"
#include <QtWidgets/QMessageBox>
#include <QFileDialog>
#include <QTextCodec>
#include <windows.h>
#include <fstream>
#include "RZCamera.h"
#include "api.h"
#include "USBControlF60.h"
#include "common.h"
#include "event.h"
#include "parse.h"

#pragma comment(lib, "netapi32.lib")

boost::mutex QtDemo::mtx;
int QtDemo::connected = -1;

bool GetMoudulePath(std::string& path)
{
    char file_path[_MAX_PATH] = { 0 };
    if (GetModuleFileName(NULL, file_path, _MAX_FNAME) == 0)
        return false;

    std::string file_path_str = file_path;
    size_t last_slash = file_path_str.find_last_of("\\");
    if (last_slash == std::string::npos)
        return false;

    path = file_path_str.substr(0, last_slash + 1);
    return true;
}

int QtDemo::ConnectCallBack(const char* path, unsigned int msg)
{
    switch (msg) {
    case 0:
    {
        ::FCloseDev();
        char buf[1024] = { 0 };
        sprintf(buf, "QtDemo::DevConnCallBack->断开, 关闭设备");
    }
        break;//设备断开连接，处理在回调函数
    case 1:
    {//设备连接	
        ::FOpenDev(path);
        char buf[1024] = { 0 };
        sprintf(buf, "QtDemo::DevConnCallBack->连接, 打开设备");
    }
    break;
    default:
        break;
    }

    mtx.lock();
    connected = msg;
    mtx.unlock();
    return 0;
}

int QtDemo::DevMsgCallBack(unsigned int uMsg, unsigned int wParam, long lParam, 
                            unsigned char* data, unsigned char len)
{
    if (STAMPER_COMPLETE == uMsg) {
    }

    if (STAMPER_PAPERDOORCLOSE == uMsg) {
        std::string str = wParam == 0 ? "关闭" : "打开";
    }

    if (STAMPER_ELEC_LOCK == uMsg) {
    }

    if (STAMPER_SIDEDOOR_CLOSE == uMsg) {
        std::string str = wParam == 0 ? "关闭" : "打开";
    }

    return 0;
}

void QtDemo::HandleConnect(const char* path, unsigned int msg)
{
    switch (msg) {
    case 0:
        Info("设备断开");
        break;//设备断开连接，处理在回调函数
    case 1:
    {//设备连接	
        //更新界面
        Info("设备打开成功");
    }
    break;
    default:
        break;
    }
}

QtDemo::QtDemo(QWidget *parent)
    : QMainWindow(parent), echo_thread_(NULL), open_called(true), test_mode_(false),
    img_(NULL), serial_(0), capture_suc(false), which_cam_(0)
{
    ui.setupUi(this);
    setWindowTitle(DIALOG_HEADER);

    register_conn_cb_ = F_RegisterDevCallBack(QtDemo::ConnectCallBack);
    register_msg_cb_ = FRegisterDevCallBack(QtDemo::DevMsgCallBack);

    connect(this, SIGNAL(ConnectStatus(const char*, unsigned int)), this, 
        SLOT(HandleConnect(const char*, unsigned int)));

    //主界面
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(HandleTabChange(int)));

    // 设备控制页
    connect(ui.pb_beep_on_, &QPushButton::clicked, this, &QtDemo::HandleBeepOn);
    connect(ui.pb_beep_off_, &QPushButton::clicked, this, &QtDemo::HandleBeepOff);

    connect(ui.pb_paper_status_, &QPushButton::clicked, this, &QtDemo::HandleReadPaper);

    connect(ui.pb_open_safe_door_alarm_, &QPushButton::clicked, this, &QtDemo::HandleOpenSafeDoorAlarm);
    connect(ui.pb_close_safe_door_alarm_, &QPushButton::clicked, this, &QtDemo::HandleCloseSafeDoorAlarm);
    connect(ui.pb_open_vibration_alarm_, &QPushButton::clicked, this, &QtDemo::HandleOpenVibrationAlarm);
    connect(ui.pb_close_vibration_alarm_, &QPushButton::clicked, this, &QtDemo::HandleCloseVibrationAlarm);

    connect(ui.pb_get_sn_, &QPushButton::clicked, this, &QtDemo::HandleQuerySN);
    connect(ui.pb_set_sn_, &QPushButton::clicked, this, &QtDemo::HandleSetSN);

    connect(ui.pb_open_safe_door_, &QPushButton::clicked, this, &QtDemo::HandleOpenSafeDoor);
    connect(ui.pb_close_safe_door_, &QPushButton::clicked, this, &QtDemo::HandleCloseSafeDoor);
    connect(ui.pb_safe_status_, &QPushButton::clicked, this, &QtDemo::HandleReadSafe);

    connect(ui.pb_read_local_mac_, &QPushButton::clicked, this, &QtDemo::HandleReadLocalMac);
    connect(ui.pb_binding_mac_, &QPushButton::clicked, this, &QtDemo::HandleBinding);
    connect(ui.pb_mac_unbinding_, &QPushButton::clicked, this, &QtDemo::HandleUnbinding);
    connect(ui.pb_query_mac_, &QPushButton::clicked, this, &QtDemo::HandleQueryMAC);
   
    connect(ui.pb_query_slots_, &QPushButton::clicked, this, &QtDemo::HandleQuerySlots);
    connect(ui.pb_query_stamper_, &QPushButton::clicked, this, &QtDemo::HandleQueryStampers);
    
    connect(ui.pb_cali_stamp_, &QPushButton::clicked, this, &QtDemo::HandleABCCheck);
    ui.groupBox_8->hide();
    ui.label->hide();
    ui.le_stamper_idx_->hide();
    ui.pb_cali_stamp_->hide();

    connect(ui.le_err_code_, SIGNAL(textChanged(const QString &)), this, 
        SLOT(HandleErrCodeChange(const QString &)));

    connect(ui.pb_cnn_status_, &QPushButton::clicked, this, &QtDemo::HandleCnnStatus);
    connect(ui.pb_open_, &QPushButton::clicked, this, &QtDemo::HandleOpenCnn);
    connect(ui.pb_close_, &QPushButton::clicked, this, &QtDemo::HandleCloseCnn);
    connect(ui.pb_open_paper_, &QPushButton::clicked, this, &QtDemo::HandleOpenPaper);

    connect(ui.pb_open_paper_led_, &QPushButton::clicked, this, &QtDemo::HandleOpenPaperLED);
    connect(ui.pb_close_paper_led_, &QPushButton::clicked, this, &QtDemo::HandleClosePaperLED);
    connect(ui.pb_open_safe_led_, &QPushButton::clicked, this, &QtDemo::HandleOpenSafeLED);
    connect(ui.pb_close_safe_led_, &QPushButton::clicked, this, &QtDemo::HandleCloseSafeLED);

    connect(ui.pb_set_resolution_, &QPushButton::clicked, this, &QtDemo::HandleSetResolution);
    connect(ui.pb_set_dpi_, &QPushButton::clicked, this, &QtDemo::HandleSetDPI);
    connect(ui.pb_start_record_, &QPushButton::clicked, this, &QtDemo::HandleStartRecord);
    connect(ui.pb_stop_record_, &QPushButton::clicked, this, &QtDemo::HandleStopRecord);

    connect(ui.pb_dev_type_, &QPushButton::clicked, this, &QtDemo::HandleGetModelType);
    connect(ui.pb_dev_status_, &QPushButton::clicked, this, &QtDemo::HandleGetDevStatus);


    //拍照/识别
    ui.pb_ori_img_->setStyleSheet("background-color: transparent;");
    ui.pb_cut_img_->setStyleSheet("background-color: transparent;");
    connect(ui.pb_ori_img_, &QPushButton::clicked, this, &QtDemo::HandleOriImgClick);
    connect(ui.pb_cut_img_, &QPushButton::clicked, this, &QtDemo::HandleCutImgClick);

    connect(ui.pb_capture_, &QPushButton::clicked, this, &QtDemo::HandleCapture);
    connect(ui.pb_select_picture_, &QPushButton::clicked, this, &QtDemo::HandleSelectPic);

    connect(ui.pb_illustrate_, &QPushButton::clicked, this, &QtDemo::HandleIllusrate);
    //connect(ui.pb_check_stamp_, &QPushButton::clicked, this, &QtDemo::HandleCheckStamp);
    ui.pb_check_stamp_->hide();

    connect(ui.combo_img_sel_, SIGNAL(activated(int)), this, SLOT(HandleSelectImg(int)));
    connect(ui.pb_recog_code_, &QPushButton::clicked, this, &QtDemo::HandleRecogCode);
    connect(ui.pb_recog_ele_, &QPushButton::clicked, this, &QtDemo::HandleRecogEle);

    connect(ui.cb_cam_list_, SIGNAL(activated(int)), this, SLOT(HandleCamListChange(int)));
    connect(ui.pb_open_cam_, &QPushButton::clicked, this, &QtDemo::HandleOpenCam);
    connect(ui.pb_close_cam_, &QPushButton::clicked, this, &QtDemo::HandleCloseCam);
    connect(ui.pb_status_cam_, &QPushButton::clicked, this, &QtDemo::HandleQueryCam);

    //盖章
    connect(ui.radio_stamper1_, SIGNAL(pressed()), this, SLOT(Stamper1()));
    connect(ui.radio_stamper2_, SIGNAL(pressed()), this, SLOT(Stamper2()));
    connect(ui.radio_stamper3_, SIGNAL(pressed()), this, SLOT(Stamper3()));
    connect(ui.radio_stamper4_, SIGNAL(pressed()), this, SLOT(Stamper4()));
    connect(ui.radio_stamper5_, SIGNAL(pressed()), this, SLOT(Stamper5()));
    connect(ui.radio_stamper6_, SIGNAL(pressed()), this, SLOT(Stamper6()));

    connect(ui.pb_stamp_read_stampers_, &QPushButton::clicked, this, &QtDemo::HandleStampReadStampers);
    connect(ui.cb_stamp_stamp_ink_, SIGNAL(stateChanged(int)), this, SLOT(HandleCheckStampInk(int)));

    connect(ui.pb_prepare_, &QPushButton::clicked, this, &QtDemo::HandlePrepare);
    connect(ui.pb_ordinary_stamp_, &QPushButton::clicked, this, &QtDemo::HandleOridinary);
    //connect(ui.pb_auto_stamp_, &QPushButton::clicked, this, &QtDemo::HandleAutoStamp);
    ui.pb_auto_stamp_->hide();

    connect(ui.pb_finish_stamp_, &QPushButton::clicked, this, &QtDemo::HandleFinishStamp);
    connect(ui.pb_release_machine_, &QPushButton::clicked, this, &QtDemo::HandleReleaseMachine);
    ui.pb_release_machine_->hide();

    // 其他接口
    connect(ui.pb_lock_, &QPushButton::clicked, this, &QtDemo::HandleLock);
    connect(ui.pb_unlock_, &QPushButton::clicked, this, &QtDemo::HandleUnlock);
    connect(ui.pb_stamper_lock_, &QPushButton::clicked, this, &QtDemo::HandleIsLock);

    connect(ui.pb_set_sidedoor_, &QPushButton::clicked, this, &QtDemo::HandleSetSideDoor);

    connect(ui.pb_check_param_, &QPushButton::clicked, this, &QtDemo::HandleCheckParam);

    connect(ui.pb_read_alarm_, &QPushButton::clicked, this, &QtDemo::HandleReadAlarm);

    connect(ui.pb_read_rfid_, &QPushButton::clicked, this, &QtDemo::HandleReadRFID);
    
    ui.cb_cam_list_->addItem(QString::fromLocal8Bit("凭证摄像头"));
    ui.cb_cam_list_->addItem(QString::fromLocal8Bit("环境摄像头"));
    ui.cb_cam_list_->addItem(QString::fromLocal8Bit("侧门摄像头"));

    ui.le_x_in_dev_->setText(QString::number(100));
    ui.le_y_in_dev_->setText(QString::number(100));

    std::string module_path;
    GetMoudulePath(module_path);
    module_path.append("record1.avi");
    ui.le_video_path_->setText(QString::fromStdString(module_path));

    // 默认显示第一个tab
    ui.tabWidget->setCurrentIndex(0);
    this->setMouseTracking(true);

    status_label_ = new QLabel(this);
    ui.statusBar->addWidget(status_label_);
    ui.statusBar->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
}

void QtDemo::HandleTabChange(int index)
{
    switch (index) {
    case 0:
        InitDevControl();
        break;
    case 1:
        InitSnapshot();
        break;
    case 2:
        InitStamp();
        break;
    case 3:
        InitOther();
        break;
    default:
        break;
    }
}

bool QtDemo::Init()
{
    if (!Config::GetInst()->Parse())
        return false;

    InitDevControl(); //需要手动调用一次
    return true;
}

QtDemo::~QtDemo()
{
}

/////////////////////////// 设备控制 ////////////////////////////////

void QtDemo::InitDevControl()
{
    ui.le_stamper_idx_->setText(QString::number(1)); //从1c
}

std::string QtDemo::GetLocalMAC()
{
    typedef  struct  _ASTAT_ {
        ADAPTER_STATUS adapt;
        NAME_BUFFER    NameBuff[30];
    } ASTAT, *PASTAT;

    ASTAT Adapter;

    NCB Ncb;
    UCHAR uRetCode;
    LANA_ENUM   lenum;
    int       i;
    memset(&Ncb, 0, sizeof(Ncb));
    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_buffer = (UCHAR  *)& lenum;
    Ncb.ncb_length = sizeof(lenum);
    uRetCode = Netbios(&Ncb);
    char mac[256] = { 0 };
    for (i = 0; i < lenum.length; i++) {
        memset(&Ncb, 0, sizeof(Ncb));
        Ncb.ncb_command = NCBRESET;
        Ncb.ncb_lana_num = lenum.lana[i];
        uRetCode = Netbios(&Ncb);
        //printf(  " The NCBRESET on LANA %d return code is: 0x%x \n " ,
        //	lenum.lana[i], uRetCode );
        memset(&Ncb, 0, sizeof(Ncb));
        Ncb.ncb_command = NCBASTAT;
        Ncb.ncb_lana_num = lenum.lana[i];
        strcpy((char   *)Ncb.ncb_callname, "*");
        Ncb.ncb_buffer = (unsigned  char   *)& Adapter;
        Ncb.ncb_length = sizeof(Adapter);
        uRetCode = Netbios(&Ncb);
        //printf(  " The NCBASTAT on LANA %d return code is: 0x%x \n " ,	lenum.lana[i], uRetCode );
        if (uRetCode == 0) {
            sprintf_s(mac, "%2x:%2x:%2x:%2x:%2x:%2x", 
                Adapter.adapt.adapter_address[0],
                Adapter.adapt.adapter_address[1],
                Adapter.adapt.adapter_address[2],
                Adapter.adapt.adapter_address[3],
                Adapter.adapt.adapter_address[4],
                Adapter.adapt.adapter_address[5]);
            //break;
        }
    }

    return std::string(mac);
}

void QtDemo::HandleReadLocalMac()
{
    std::string mac = GetLocalMAC();
    if (mac.empty())
        return Info(QString::fromLocal8Bit("获取本机MAC地址失败"));

    ui.le_mac_->setText(QString::fromStdString(mac));
    ui.statusBar->showMessage(QString::fromLocal8Bit("成功获取本机MAC地址"), STATUS_TEXT);
}

void QtDemo::HandleBinding()
{
    std::string mac = ui.le_mac_->text().toStdString();
    if (mac.empty())
        return Info(QString::fromLocal8Bit("待绑定MAC地址为空"));

    int ret = ST_BindMAC(mac);
    return Info(QString::fromLocal8Bit("绑定MAC地址, er: ") + QString::number(ret));
}

void QtDemo::HandleUnbinding()
{
    std::string mac = ui.le_mac_->text().toStdString();
    if (mac.empty())
        return Info(QString::fromLocal8Bit("待解绑MAC地址为空"));

    int ret = ST_UnbindMAC(mac);
    return Info(QString::fromLocal8Bit("解绑MAC地址, er: ") + QString::number(ret));
}

void QtDemo::HandleQueryMAC()
{
    std::string mac1;
    std::string mac2;
    int ret = ST_QueryMAC(mac1, mac2);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("查询MAC地址失败, er: ") + QString::number(ret));

    ui.le_mac1_->clear();
    ui.le_mac2_->clear();

    ui.le_mac1_->setText(QString::fromStdString(mac1));
    ui.le_mac2_->setText(QString::fromStdString(mac2));
}

void QtDemo::HandleOpenSafeDoorAlarm()
{
    int ret = ST_ControlAlarm(0, 1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开安全门报警失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功打开安全门报警"), STATUS_TEXT);
}

void QtDemo::HandleCloseSafeDoorAlarm()
{
    int ret = ST_ControlAlarm(0, 0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭安全门报警失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功关闭安全门报警"), STATUS_TEXT);
}

void QtDemo::HandleOpenVibrationAlarm()
{
    int ret = ST_ControlAlarm(1, 1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开振动报警失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功打开振动报警"), STATUS_TEXT);
}

void QtDemo::HandleCloseVibrationAlarm()
{
    int ret = ST_ControlAlarm(1, 0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭振动报警失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功关闭振动报警"), STATUS_TEXT);
}

void QtDemo::HandleQuerySN()
{
    std::string sn;
    int ret = ST_QueryMachine(sn);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("获取设备编号失败, er: ") + QString::number(ret));

    ui.le_show_sn_->clear();
    ui.le_show_sn_->setText(QString::fromStdString(sn));
}

void QtDemo::HandleSetSN()
{
    std::string sn = ui.le_set_sn_->text().toStdString();
    if (sn.empty())
        return Info(QString::fromLocal8Bit("设备编号为空"));

    int ret = ST_SetMachine(sn);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("设置设备编号失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置设备编号成功"), STATUS_TEXT);
}

void QtDemo::HandleCnnStatus()
{
    int cnn;
    int ret = ST_QueryCnn(cnn);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("获取连接状态失败, er: ") + 
            QString::number(ret));
        
    if (1 == cnn)
        ui.statusBar->showMessage(QString::fromLocal8Bit("设备连接"), STATUS_TEXT);
    else
        ui.statusBar->showMessage(QString::fromLocal8Bit("设备断开"), STATUS_TEXT);
}

void QtDemo::HandleOpenCnn()
{
    int ret = ST_Open();
    if (0 != ret)
        return Info(QString::fromLocal8Bit("打开设备失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("打开设备成功"), STATUS_TEXT);
}

void QtDemo::HandleCloseCnn()
{
    int ret = ST_Close();
    if (0 != ret)
        return Info(QString::fromLocal8Bit("关闭设备失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关闭设备成功"), STATUS_TEXT);
}

void QtDemo::HandleOpenPaper()
{
    int timeout = atoi(ui.le_paper_timeout_->text().toStdString().c_str());
    int ret = ST_OpenPaper(timeout);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("开进纸门失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("打开纸门成功"), STATUS_TEXT);
}

void QtDemo::HandleOpenPaperLED()
{
    int value = atoi(ui.le_led_val_->text().toStdString().c_str());
    int ret = ST_ControlLed(2, 1, value);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("开凭证补光灯失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("开凭证补光灯成功"), STATUS_TEXT);
}

void QtDemo::HandleClosePaperLED()
{
    int ret = ST_ControlLed(2, 0, 0);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("关凭证补光灯失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关凭证补光灯成功"), STATUS_TEXT);
}

void QtDemo::HandleOpenSafeLED()
{
    int value = atoi(ui.le_led_val_->text().toStdString().c_str());
    int ret = ST_ControlLed(1, 1, value);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("开安全门补光灯失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("开安全门补光灯成功"), STATUS_TEXT);
}

void QtDemo::HandleCloseSafeLED()
{
    int ret = ST_ControlLed(1, 0, 0);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("关安全门补光灯失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关安全门补光灯成功"), STATUS_TEXT);
}

void QtDemo::HandleSetResolution()
{
    Info(QString::fromLocal8Bit("请确保摄像头已经打开"));

    int width = atoi(ui.le_cam_width_->text().toStdString().c_str());
    int height = atoi(ui.le_cam_height_->text().toStdString().c_str());
    int ret = ST_SetResolution(which_cam_, width, height);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("设置摄像头分辨率失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置摄像头分辨率成功"), STATUS_TEXT);
}

void QtDemo::HandleSetDPI()
{
    Info(QString::fromLocal8Bit("请确保摄像头已经打开"));

    int width = atoi(ui.le_cam_width_->text().toStdString().c_str());
    int height = atoi(ui.le_cam_height_->text().toStdString().c_str());
    int ret = ST_SetDPIValue(which_cam_, width, height);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("设置DPI失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置DPI成功"), STATUS_TEXT);
}

void QtDemo::HandleStartRecord()
{
    std::string path = ui.le_video_path_->text().toStdString();
    int ret = ST_StartRecordVideo(which_cam_, path);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("开始录制视频失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("开始录制视频成功"), STATUS_TEXT);
}

void QtDemo::HandleStopRecord()
{
    std::string path = ui.le_video_path_->text().toStdString();
    int ret = ST_StopRecordVideo(which_cam_, path);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("停止录制视频失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("停止录制视频成功"), STATUS_TEXT);
}

void QtDemo::HandleGetModelType()
{
    std::string type;
    int ret = ST_GetDevModel(type);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("获取设备型号失败, er: ") +
            QString::number(ret));

    Info(QString::fromLocal8Bit("设备型号: ") + QString::fromStdString(type));
}

void QtDemo::HandleOpenSafeDoor()
{
    int ctrl = 1;
    int ret = ST_ControlSafe(ctrl);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("打开安全门失败, er: ") + 
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("打开安全门成功"), STATUS_TEXT);
}

void QtDemo::HandleCloseSafeDoor()
{
    int ctrl = 0;
    int ret = ST_ControlSafe(ctrl);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("关闭安全门失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关闭安全门成功"), STATUS_TEXT);
}

void QtDemo::HandleReadSafe()
{
    int safe_status;
    int ret2 = ST_QuerySafe(safe_status);
    if (ret2 != 0)
        return Info(QString::fromLocal8Bit("获取安全门状态失败, er: ") + QString::number(ret2));

    if (0 == safe_status)
        Info(QString::fromLocal8Bit("安全门关闭"));
    else if (1 == safe_status)
        Info(QString::fromLocal8Bit("安全门打开"));
    else
        Info(QString::fromLocal8Bit("安全门处于未知状态"));
}

void QtDemo::HandleReadPaper()
{
    int paper_status;
    int ret1 = ST_QueryPaper(paper_status);
    if (ret1 != 0)
        return Info(QString::fromLocal8Bit("获取进纸门状态失败, er: ") + QString::number(ret1));

    if (0 == paper_status)
        Info(QString::fromLocal8Bit("进纸门关闭"));
    else if (1 == paper_status)
        Info(QString::fromLocal8Bit("进纸门打开"));
    else
        Info(QString::fromLocal8Bit("进纸门处于未知状态"));
}

void QtDemo::HandleBeepOn()
{
    int ret = ST_ControlBeep(1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开蜂鸣器失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("打开蜂鸣器成功"), STATUS_TEXT);
}

void QtDemo::HandleBeepOff()
{
    int ret = ST_ControlBeep(0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭蜂鸣器失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关闭蜂鸣器成功"), STATUS_TEXT);
}

void QtDemo::HandleQueryStampers()
{
    char stampers[7] = { 0 };
    int ret = ST_QueryStampers(stampers, 7);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("章状态查询失败, err: ") +
            QString::number(ret));

    Info(QString::fromLocal8Bit("章状态: ") + QString::fromStdString(stampers));
}

void QtDemo::HandleQuerySlots()
{
    int num;
    int ret = ST_QuerySlots(num);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("查询卡槽数量失败, err: ") +
            QString::number(ret));

    Info(QString::fromLocal8Bit("卡槽数量: ") + QString::number(num));
}

void QtDemo::HandleABCCheck()
{
    std::string str = ui.le_stamper_idx_->text().toStdString();
    /*int ret = ST_Calibrate(atoi(str.c_str()));*/
//     if (0 != ret) {
//         return Info(QString::fromLocal8Bit("校准印章失败, er: ") + QString::number(ret));
//     }

    ui.statusBar->showMessage(QString::fromLocal8Bit("校准印章成功"), STATUS_TEXT);
    return;

    //////////////////////////////////////////////////////////////////////////

    unsigned int rfid = 0;
    int ret = GetStamperID(atoi(str.c_str()) - 1, rfid);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取印章ID失败"));

    char stamper_id[4] = { 0 };
    memcpy(stamper_id, &rfid, sizeof(rfid));
    short pts[] =
    {
        12, 34,
        54, 104,
        209, 45,
        232, 13,
        101, 110
    };

    ret = CalibrationEx(stamper_id, (char*)pts, 10);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("农行校准失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("农行校准成功"), STATUS_TEXT);
}

void QtDemo::HandleErrCodeChange(const QString & txt)
{
    std::string err_str = txt.toStdString();
    for (int i = 0; i < err_str.length(); ++i) {
        if (err_str.at(i) < 0x30 || err_str.at(i) > 0x39)
            return Info(QString::fromLocal8Bit("错误码应为正整数, 请重新输入"));
    }

    int err = atoi(err_str.c_str());
    std::string msg, resolver;
    int ret = ST_GetError(err, msg, resolver);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("查询错误码失败"));

    ui.le_show_err_msg_->setText(QString::fromLocal8Bit("错误信息: ") + 
        QString::fromLocal8Bit(msg.c_str()));

    ui.le_show_err_resolver_->setText(QString::fromLocal8Bit("解决方案: ") +
        QString::fromLocal8Bit(resolver.c_str()));
}

/////////////////////////////// 盖章 /////////////////////////////////////

void QtDemo::HandleOriImgClick()
{
    if (!capture_suc)
        return;

    QPoint click_pos = ui.pb_ori_img_->mapFromGlobal(QCursor::pos());

    int content_width = ui.ori_image_->contentsRect().width() + 1;
    int pixmap_width = ui.ori_image_->pixmap()->rect().width();

    int content_height = ui.ori_image_->contentsRect().height() + 1;
    int pixmap_height = ui.ori_image_->pixmap()->rect().height();

    int x_in_ori = pixmap_width * click_pos.x() / content_width;
    int y_in_ori = pixmap_height * click_pos.y() / content_height;

    ui.le_x_in_img_->clear();
    ui.le_y_in_img_->clear();
    ui.le_x_in_img_->setText(QString::number(x_in_ori));
    ui.le_y_in_img_->setText(QString::number(y_in_ori));

    ui.statusBar->showMessage(
        QString::fromLocal8Bit("clicked: ") +
        QString::number(click_pos.x()) + "," + QString::number(click_pos.y()) +
        QString::fromLocal8Bit(", In ori: ") + 
        QString::number(x_in_ori) + ", " + QString::number(y_in_ori), STATUS_TEXT);
}

void QtDemo::HandleCutImgClick()
{
    if (!capture_suc)
        return;

    mouse_press_pt_ = QCursor::pos();

    QPoint click_pos = ui.pb_cut_img_->mapFromGlobal(QCursor::pos());

    int content_width = ui.cut_image_->contentsRect().width() + 1;
    int pixmap_width = ui.cut_image_->pixmap()->rect().width();

    int content_height = ui.cut_image_->contentsRect().height() + 1;
    int pixmap_height = ui.cut_image_->pixmap()->rect().height();

    int x_in_ori = pixmap_width * click_pos.x() / content_width;
    int y_in_ori = pixmap_height * click_pos.y() / content_height;

    ui.le_x_in_img_->clear();
    ui.le_y_in_img_->clear();
    ui.le_x_in_img_->setText(QString::number(x_in_ori));
    ui.le_y_in_img_->setText(QString::number(y_in_ori));

    ui.statusBar->showMessage(
        QString::fromLocal8Bit("clicked: ") +
        QString::number(click_pos.x()) + "," + QString::number(click_pos.y()) +
        QString::fromLocal8Bit(", In ori: ") +
        QString::number(x_in_ori) + ", " + QString::number(y_in_ori), STATUS_TEXT);
}

void QtDemo::mousePressEvent(QMouseEvent * ev)
{
    mouse_press_pt_ = QCursor::pos();
    ui.statusBar->showMessage(QString::fromLocal8Bit("mouse press event (") +
        QString::number(mouse_press_pt_.x()) + ", " + 
        QString::number(mouse_press_pt_.y()) + ")", STATUS_TEXT);
}

void QtDemo::mouseReleaseEvent(QMouseEvent *)
{
    ui.statusBar->showMessage(QString::fromLocal8Bit("mouse release event"), STATUS_TEXT);
}

void QtDemo::mouseMoveEvent(QMouseEvent *)
{
    mouse_moving_pt_ = QCursor::pos();

    ui.statusBar->showMessage(QString::fromLocal8Bit("mouse move event (") + 
        QString::number(mouse_moving_pt_.x()) + ", " +
        QString::number(mouse_moving_pt_.y()) + ")", STATUS_TEXT);
}

void QtDemo::HandleCapture()
{
    const std::string ori_path = Config::GetInst()->ori_path_;
    const std::string cut_path = Config::GetInst()->cut_path_;
    int ret = ST_Snapshot(
        which_cam_,
//         200,
//         200,
        ori_path,
        cut_path);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("拍照失败, err: ") + QString::number(ret));

    ui.le_ori_path_->clear();
    ui.le_cut_path_->clear();
    ui.ori_image_->clear();
    ui.cut_image_->clear();

    QImage ori_img;
    if (ori_img.load(ori_path.c_str())) {
        QPixmap pix = QPixmap::fromImage(ori_img);
        ui.ori_image_->setAlignment(Qt::AlignCenter);
        ui.ori_image_->setScaledContents(true);
        ui.ori_image_->setPixmap(pix);
        ui.ori_image_->show();

        capture_suc = true;
    } else {
        capture_suc = false;
    }

    QImage cut_img;
    if (cut_img.load(cut_path.c_str())) {
        QPixmap pix = QPixmap::fromImage(cut_img);
        ui.cut_image_->setAlignment(Qt::AlignCenter);
        ui.cut_image_->setScaledContents(true);
        ui.cut_image_->setPixmap(pix);
        ui.cut_image_->show();

        capture_suc = true;
    } else {
        capture_suc = false;
    }

    if (capture_suc) {
        ui.le_ori_path_->setText(QString::fromStdString(ori_path));
        ui.le_cut_path_->setText(QString::fromStdString(cut_path));
        Info(QString::fromLocal8Bit("拍照并加载图片成功"));
    } else {
        Info(QString::fromLocal8Bit("加载显示图片成功"));
    }
}

void QtDemo::HandleSelectPic()
{
    QFileDialog* dlg = new QFileDialog(this, "File Dialog", ".");
    if (dlg->exec() == QFileDialog::Accepted) {
        //QString file = dlg->fileSelected();
    }

    QStringList filenames = QFileDialog::getOpenFileNames(
        this, 
        tr("BMP files"),
        QDir::currentPath(), 
        tr("Bitmap files (*.bmp);;All files (*.*)"));
    QString str;
    if (!filenames.isEmpty())
    {
        for (int i = 0; i < filenames.count(); i++)
            str.append(filenames.at(i));
    }
    Info(str);
}

void QtDemo::HandleIllusrate()
{
    QImage cut_img;
    if (cut_img.load(Config::GetInst()->cut_path_.c_str())) {
        QPixmap pix = QPixmap::fromImage(cut_img);
        QPainter painter(&pix);
        painter.begin(this);
        painter.setPen(QPen(Qt::red, 4, Qt::SolidLine));    // 设置画笔形式 
        painter.setBrush(QBrush(Qt::red, Qt::NoBrush));     // 设置画刷形式
        painter.drawRect(
            atoi(ui.le_x_in_img_->text().toStdString().c_str()),
            atoi(ui.le_y_in_img_->text().toStdString().c_str()),
            atoi(ui.le_input_width_->text().toStdString().c_str()),
            atoi(ui.le_input_height_->text().toStdString().c_str()));
        painter.end();

        ui.cut_image_->setAlignment(Qt::AlignCenter);
        ui.cut_image_->setScaledContents(true);
        ui.cut_image_->setPixmap(pix);
        ui.cut_image_->show();
    }
}

void QtDemo::HandleCheckStamp()
{
    if (0 == para_.stamp_idx) {
        Info(QString::fromLocal8Bit("请在\"盖章页\"先选章, 再盖章"));
        return;
    }

    FOpenDev(NULL);
    unsigned int rfid;
    int ret = GetStamperID(para_.stamp_idx - 1, rfid);

    STAMPERPARAM pa;
    memcpy(&pa.seal, &rfid, sizeof(rfid));
    pa.serial_number = GetTickCount();
    pa.isPadInk = 1;
    pa.x_point = atoi(ui.le_x_in_dev_->text().toStdString().c_str());
    pa.y_point = atoi(ui.le_y_in_dev_->text().toStdString().c_str());
    pa.w_time = 2;
    pa.angle = 0;
    pa.type = 0;

    ret = FStartStamperstrc(&pa);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("盖章失败, er: ") + QString::number(ret));

    if (ui.le_x_in_dev_->text().toStdString() == "3" &&
        ui.le_y_in_dev_->text().toStdString() == "60") {
        ui.le_x_in_dev_->setText(QString::number(270));
        ui.le_y_in_dev_->setText(QString::number(250));
    } else {
        ui.le_x_in_dev_->setText(QString::number(3));
        ui.le_y_in_dev_->setText(QString::number(60));
    }
}

void QtDemo::HandleSelectImg(int index)
{
    std::string img = ui.combo_img_sel_->currentText().toStdString();
    img_type_ = index;
}

void QtDemo::HandleCamListChange(int index)
{
    which_cam_ = index;
}

void QtDemo::HandleRecogCode()
{
    // 清空之前结果
    ui.la_code_result_->clear();

    std::string template_id;
    std::string trace_num;
    int ret;
    if (0 == img_type_) {
        ret = ST_RecognizeImage(Config::GetInst()->ori_path_, template_id, trace_num);
    }
    else if (1 == img_type_) {
        ret = ST_RecognizeImage(Config::GetInst()->cut_path_, template_id, trace_num);
    }

    if (0 != ret)
        return Info(QString::fromLocal8Bit("版面、验证码识别失败, er: ") + QString::number(ret));

    ui.la_code_result_->setText(QString::fromLocal8Bit("模板ID: ") + QString::fromStdString(template_id) +
        QString::fromLocal8Bit("\n追溯码: ") + QString::fromStdString(trace_num));
    ui.la_code_result_->adjustSize();
}

void QtDemo::HandleRecogEle()
{
    ui.le_ele_result_->clear();

    std::string img_path;
    if (0 == img_type_)
        img_path = Config::GetInst()->ori_path_;
    else
        img_path = Config::GetInst()->cut_path_;

    std::string result;
    int ret = ST_IdentifyElement(
        img_path,
        atoi(ui.le_x_in_img_->text().toStdString().c_str()),
        atoi(ui.le_y_in_img_->text().toStdString().c_str()),
        atoi(ui.le_input_width_->text().toStdString().c_str()),
        atoi(ui.le_input_height_->text().toStdString().c_str()),
        0,
        result);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("要素识别失败, er: ") + QString::number(ret));

    ui.le_ele_result_->setText(QString::fromStdString(result));
    ui.le_ele_result_->adjustSize();
    ui.statusBar->showMessage(QString::fromLocal8Bit("要素识别成功"), STATUS_TEXT);
}

void QtDemo::HandleOpenCam()
{
    int ret = ST_OpenCamera(which_cam_);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("打开摄像头-") + 
            QString::number(which_cam_) +
            ui.cb_cam_list_->currentText() +
            QString::fromLocal8Bit("-失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("打开摄像头成功"), STATUS_TEXT);
}

void QtDemo::HandleCloseCam()
{
    int ret = ST_CloseCamera(which_cam_);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("关闭摄像头-") +
            QString::number(which_cam_) +
            ui.cb_cam_list_->currentText() +
            QString::fromLocal8Bit("-失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关闭摄像头成功"), STATUS_TEXT);
}

void QtDemo::HandleQueryCam()
{
    int status;
    int ret = ST_QueryCamera(which_cam_, status);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("摄像头状态-") +
            QString::number(which_cam_) +
            ui.cb_cam_list_->currentText() +
            QString::fromLocal8Bit("-失败, er: ") +
            QString::number(ret));

    Info(ui.cb_cam_list_->currentText() + QString::fromLocal8Bit(": ") +
        QString::fromLocal8Bit(status == 0 ? "关闭" : "打开"));
}

void QtDemo::HandlePreStamp()
{

}

void QtDemo::InitSnapshot()
{
    ui.combo_img_sel_->clear();
    ui.combo_img_sel_->addItem(QString::fromLocal8Bit("原图"));
    ui.combo_img_sel_->addItem(QString::fromLocal8Bit("切图"));
    ui.combo_img_sel_->setFixedSize(111, 22);
// 
//     ui.le_x_in_dev_->setText(QString::number(3));
//     ui.le_y_in_dev_->setText(QString::number(60));

    ui.pb_select_picture_->hide();
}

////////////////////////// EEPROM存储 ////////////////////////////////////

bool QtDemo::IsOpened()
{
    if (open_called)
        return true;

    QMessageBox::information(
        NULL,
        DIALOG_HEADER,
        cmd_des[OC_DEV_NOT_OPENED],
        QMessageBox::No,
        QMessageBox::NoButton);
    return false;
}

///////////////////////////// 盖章操作 /////////////////////////////

void QtDemo::Stamper1()
{
    ui.radio_stamper1_->setChecked(true);
    para_.stamp_idx = 1;
}

void QtDemo::Stamper2()
{
    ui.radio_stamper2_->setChecked(true);
    para_.stamp_idx = 2;
}

void QtDemo::Stamper3()
{
    ui.radio_stamper3_->setChecked(true);
    para_.stamp_idx = 3;
}

void QtDemo::Stamper4()
{
    ui.radio_stamper4_->setChecked(true);
    para_.stamp_idx = 4;
}

void QtDemo::Stamper5()
{
    ui.radio_stamper5_->setChecked(true);
    para_.stamp_idx = 5;
}

void QtDemo::Stamper6()
{
    ui.radio_stamper6_->setChecked(true);
    para_.stamp_idx = 6;
}

void QtDemo::InitStamp()
{
    ui.le_paper_door_timeout_->setText(QString::number(30));
}

//开启工厂测试模式
void QtDemo::HandleEnableFactory()
{
    if (!IsOpened())
        return;

    int ret = EnableFactoryMode(1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("开启工厂模式失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功开始工厂模式"), STATUS_TEXT);
}

//关闭工厂测试模式
void QtDemo::HandleDisableFactory()
{
    if (!IsOpened())
        return;

    int ret = EnableFactoryMode(0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭工厂模式失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功关闭工厂模式"), STATUS_TEXT);
}

void QtDemo::HandleStampReadStampers()
{
    if (!IsOpened())
        return;

    FOpenDev(NULL);
    QRadioButton* btns[] =
    {
        ui.radio_stamper1_,
        ui.radio_stamper2_,
        ui.radio_stamper3_,
        ui.radio_stamper4_,
        ui.radio_stamper5_,
        ui.radio_stamper6_
    };

    for (int i = 0; i < 6; ++i) {
        btns[i]->setText("");
        unsigned int rfid = 0;
        int ret = GetStamperID(i, rfid);
        if (STF_SUCCESS != ret)
            continue;

        char str[1024] = { 0 };
        if (rfid == 0) {
            sprintf(str, "<%d>号章无章", i + 1);
        } else {
            base::Number2HexStr<unsigned int> rfid_hex(rfid);
            sprintf(str, "<%d>号章:0x%s", i + 1, rfid_hex.UpperHexStr().c_str());
        }
        btns[i]->setText(QString::fromLocal8Bit(str));
        btns[i]->adjustSize();
    }
}

void QtDemo::HandleCheckStampInk(int checked)
{
    bool ink = ui.cb_stamp_stamp_ink_->isChecked();
    para_.ink = ink ? 1 : 0;
}

void QtDemo::HandleOridinary()
{
    int ret = ST_OrdinaryStamp(
        ui.le_task_id_->text().toStdString(),
        para_.stamp_idx,
        para_.ink,
        atoi(ui.le_x_in_dev_->text().toStdString().c_str()),    // 盖章位置x坐标
        atoi(ui.le_y_in_dev_->text().toStdString().c_str()),    // 盖章位置y坐标
        0);                                                     // 印章旋转角度, 大于等于0且小于360度
    if (0 != ret)
        return Info(QString::fromLocal8Bit("普通用印失败,er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("普通盖章成功"), STATUS_TEXT);
}

void QtDemo::HandleAutoStamp()
{

}

// 准备用印
void QtDemo::HandlePrepare()
{
    if (0 == para_.stamp_idx) {
        Info(QString::fromLocal8Bit("请先选章, 再准备用印"));
        return;
    }

    std::string task_id;
    int time = atoi(ui.le_paper_door_timeout_->text().toStdString().c_str());
    char stamp_num = 1;
    int ret = ST_PrepareStamp(
        stamp_num,
        time,
        task_id);
    if (0 == ret) {
        ui.le_task_id_->setText(QString::fromStdString(task_id));
        ui.le_task_id_->adjustSize();
    }
    else if (2 == ret) {
        ui.le_task_id_->setText(QString::fromLocal8Bit("超时未关闭进纸门, 请重新准备"));
        ui.le_task_id_->adjustSize();
    }
    else {
        Info(QString::fromLocal8Bit("准备用印, er: ") + QString::number(ret));
    }
}

void QtDemo::HandleFinishStamp()
{
    std::string task_id = ui.le_task_id_->text().toStdString();
    if (task_id.empty()) {
        return Info(QString::fromLocal8Bit("任务号为空, 请先准备用印获取任务号"));
    }

    int ret = ST_FinishStamp(task_id);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("结束用印失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("结束用印成功"), STATUS_TEXT);
}

void QtDemo::HandleReleaseMachine()
{
    std::string machine = ui.le_show_sn_->text().toStdString();
    if (machine.empty())
        return Info(QString::fromLocal8Bit("请先通过\"设备控制\"页获取设备编号"));

//     int ret = ST_ReleaseStamp(machine);
//     if (0 != ret)
//         return Info(QString::fromLocal8Bit("释放印控机失败, er: ") + QString::number(ret));
// 
//     ui.statusBar->showMessage(QString::fromLocal8Bit("释放印控机成功"), STATUS_TEXT);
}

//////////////////////////// 其他接口 ///////////////////////////////

void QtDemo::InitOther()
{
    ui.le_sidedoor_close_->setText("30");
    ui.le_sidedoor_timeout_->setText("60");
}

void QtDemo::HandleLock()
{
    int ret = ST_Lock();
    if (0 != ret)
        return Info(QString::fromLocal8Bit("锁定印控仪失败, er: ") + 
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功锁定印控仪"), STATUS_TEXT);
}

void QtDemo::HandleUnlock()
{
    int ret = ST_Unlock();
    if (0 != ret)
        return Info(QString::fromLocal8Bit("解锁印控仪失败, er: ") + 
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功解锁印控仪"), STATUS_TEXT);
}

void QtDemo::HandleIsLock()
{
    int lock;
    int ret = ST_QueryLock(lock);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("获取锁定状态失败, er: ") + 
            QString::number(ret));

    if (1 == lock)
        Info(QString::fromLocal8Bit("未锁定"));
    else if (0 == lock)
        Info(QString::fromLocal8Bit("已锁定"));
}

void QtDemo::HandleSetSideDoor()
{
    std::string time1 = ui.le_sidedoor_close_->text().toStdString();
    std::string time2 = ui.le_sidedoor_timeout_->text().toStdString();

    int ret = ST_SetSideDoor(atoi(time1.c_str()), atoi(time2.c_str()));
    if (ret != 0)
        return Info(QString::fromLocal8Bit("设置安全门时间失败, er: ") + 
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置安全门开门成功"), STATUS_TEXT);
}

void QtDemo::ShowTimeElapsed(unsigned long begin)
{
    unsigned long end = GetTickCount();
    unsigned long interval = end - begin;
    Info(QString::fromLocal8Bit("耗时(毫秒): ") + QString::number(interval));
}

void QtDemo::HandleCheckParam()
{
    int x = atoi(ui.le_x_coord_->text().toStdString().c_str());
    int y = atoi(ui.le_y_coord_->text().toStdString().c_str());
    int angle = atoi(ui.le_angle_->text().toStdString().c_str());
    int ret = ST_CheckParam(x, y, angle);
    if (44 == ret)
        return Info(QString::fromLocal8Bit("x参数非法"));
    if (45 == ret)
        return Info(QString::fromLocal8Bit("y参数非法"));
    if (46 == ret)
        return Info(QString::fromLocal8Bit("章旋转角度非法"));

    if (0 != ret)
        return Info(QString::fromLocal8Bit("参数检查失败, er: ") +
            QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("用印参数合法"), STATUS_TEXT);
}

void QtDemo::HandleReadAlarm()
{
    int door = -1;
    int vibration = -1;
    int ret = ST_ReadAlarm(door, vibration);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("读报警器状态失败, err:") + 
            QString::number(ret));

    ui.label_door_alarm_->setText(
        0 == door ? QString::fromLocal8Bit("关"): QString::fromLocal8Bit("开"));
    ui.label_door_alarm_->adjustSize();

    ui.label_vibration_alarm_->setText(
        0 == vibration ? QString::fromLocal8Bit("关") : QString::fromLocal8Bit("开"));
    ui.label_vibration_alarm_->adjustSize();

    ui.statusBar->showMessage(QString::fromLocal8Bit("读报警器状态成功"), STATUS_TEXT);
}

void QtDemo::HandleReadRFID()
{
    int slot = atoi(ui.le_rfid_slot_->text().toStdString().c_str());
    int rfid;
    int ret = ST_GetRFID(slot, rfid);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("获取rfid失败, err:") + QString::number(ret));
    Info(QString::number(slot) + QString::fromLocal8Bit("卡槽的RFID: ") +
        QString::number(rfid));
}

//////////////////////////////////////////////////////////////////////////

//定时器到期函数运行在主线程中
void QtDemo::TimerDone()
{
    //需要加锁保护共享变量connected
    mtx.lock();
    if (connected == 0) {
        connected = -1;
        Info(QString::fromLocal8Bit("设备已断开"));
    } else if (connected == 1) {
        connected = -1;
        Info(QString::fromLocal8Bit("设备重连成功"));
    }
    else;
    mtx.unlock();

    Message* msg = NULL;
    if (msg_queue_.Pop(msg)) {
        switch (msg->what_) {
        case CMD_OPEN_DEV:
            OpenDevPost(msg);
            break;
        case CMD_CLOSE_DEV:
            CloseDevPost(msg);
            break;
        default:
            break;
        }
    }

    delete msg;
}

void QtDemo::StartTimer(uint16_t milliseconds)
{

}

void QtDemo::PushMessage(Message* msg)
{
    msg_queue_.Push(msg);
}

void QtDemo::OpenDevPost(const Message* msg)
{
    open_called = msg->err_ == 0;
}

void QtDemo::CloseDevPost(const Message* msg)
{
    open_called = msg->err_ != 0;
}

void QtDemo::HandleGetDevStatus()
{
    int code;
    int ret = ST_GetDevStatus(code);
    if (0 != ret) {
        return Info(QString::fromLocal8Bit("获取设备状态失败, err:") + 
            QString::number(ret));
    }

     // 设备状态机描述
     char* status_des[] = {
         "未初始化",
         "启动自检",
         "检测章",
         "空闲状态",
         "测试模式",
         "故障模式",
         "盖章模式",
         "维护模式"
     };
    Info(QString::fromLocal8Bit("设备状态: ") +
        QString::fromLocal8Bit(status_des[code]));
}
