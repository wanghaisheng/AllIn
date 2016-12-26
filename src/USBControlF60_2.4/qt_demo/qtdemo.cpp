#include "qtdemo.h"
#include <QtWidgets/QMessageBox>
#include <QFileDialog>
#include <QTextCodec>
#include <QDebug>
#include <windows.h>
#include <fstream>
#include "USBControlF60.h"
#include "common.h"
#include "event.h"

#pragma comment(lib, "netapi32.lib")

boost::mutex QtDemo::mtx;
int QtDemo::connected = -1;

int QtDemo::ConnectCallBack(const char* path, unsigned int msg)
{
    switch (msg) {
    case 0:
    {
        ::FCloseDev();
        char buf[1024] = { 0 };
        sprintf(buf, "QtDemo::DevConnCallBack->断开, 关闭设备");
        BOOST_LOG_TRIVIAL(debug) << buf;
    }
        break;//设备断开连接，处理在回调函数
    case 1:
    {//设备连接	
        ::FOpenDev(path);
        SetEvent(UpdateEvent);
        char buf[1024] = { 0 };
        sprintf(buf, "QtDemo::DevConnCallBack->连接, 打开设备");
        BOOST_LOG_TRIVIAL(debug) << buf;
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
        BOOST_LOG_FUNC();
        BOOST_LOG_TRIVIAL(debug) << uMsg << "," << wParam;
    }

    if (STAMPER_PAPERDOORCLOSE == uMsg) {
        std::string str = wParam == 0 ? "关闭" : "打开";
        BOOST_LOG_TRIVIAL(debug) << "进纸门" << str << " at: " << GetTickCount();

        //关闭开门报警器
        int nAlarm = ::SetAlarm(0, 0);
        BOOST_LOG_TRIVIAL(debug) << "关闭门报警[" + nAlarm;

        //关闭震动报警
        nAlarm = ::SetAlarm(1, 0);
        BOOST_LOG_TRIVIAL(debug) << "关闭震动报警[" + nAlarm;

        nAlarm = ::FBeepCtrl(0, 0);
        BOOST_LOG_TRIVIAL(debug) << "关闭蜂鸣器[" + nAlarm;
    }

    if (STAMPER_ELEC_LOCK == uMsg) {
        //QString msg = "电子锁" + QString::fromLocal8Bit("关闭");
        BOOST_LOG_TRIVIAL(debug) << "电子锁关闭";
// 
//         QMessageBox::information(
//             NULL,
//             DIALOG_HEADER,
//             msg,
//             QMessageBox::Yes,
//             QMessageBox::NoButton);
    }

    if (STAMPER_SIDEDOOR_CLOSE == uMsg) {
        std::string str = wParam == 0 ? "关闭" : "打开";
        BOOST_LOG_TRIVIAL(debug) << "安全门" << str;
    }

    return 0;
}

void QtDemo::HandleConnect(const char* path, unsigned int msg)
{
    switch (msg) {
    case 0:
        setWindowTitle("设备断开");
        ::FCloseDev();
        break;//设备断开连接，处理在回调函数
    case 1:
    {//设备连接	
        //更新界面
        setWindowTitle("设备已连接");
        ::FOpenDev(path);
        setWindowTitle("设备打开成功");
        SetEvent(UpdateEvent);
    }
    break;
    default:
        break;
    }
}

QtDemo::QtDemo(QWidget *parent)
    : QMainWindow(parent), echo_thread_(NULL), open_called(false), test_mode_(false),
    img_(NULL), serial_(0), update_thread_(NULL)
{
    ui.setupUi(this);
    setWindowTitle(DIALOG_HEADER);

    //FDebugLogSwitch(1); //debug调试信息开

    register_conn_cb_ = F_RegisterDevCallBack(QtDemo::ConnectCallBack);
    register_msg_cb_ = FRegisterDevCallBack(QtDemo::DevMsgCallBack);

    connect(this, SIGNAL(ConnectStatus(const char*, unsigned int)), this, 
        SLOT(HandleConnect(const char*, unsigned int)));

//     connect(this, SIGNAL(UpdateProgress(int, int)), this,
//         SLOT(HandleUpdate(int, int)));

    //主界面
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(HandleTabChange(int)));
    connect(ui.btn_open_dev, &QPushButton::clicked, this, &QtDemo::HandleBtnOpenDev);
    connect(ui.btn_close_dev, &QPushButton::clicked, this, &QtDemo::HandleBtnCloseDev);

    //设备控制页
    connect(ui.pb_reset_, &QPushButton::clicked, this, &QtDemo::HandleReset);
    connect(ui.pb_system_info_, &QPushButton::clicked, this, &QtDemo::HandleSystemInfo);

    connect(ui.pb_enter_maintain_mode_, &QPushButton::clicked, this, &QtDemo::HandleEnterEntainmainMode);
    connect(ui.pb_exit_maintain_mode_, &QPushButton::clicked, this, &QtDemo::HandleExitEntainmainMode);
    connect(ui.pb_version_, &QPushButton::clicked, this, &QtDemo::HandleFirewareVersion);

    connect(ui.pb_set_sn_, &QPushButton::clicked, this, &QtDemo::HandleSetSN);
    connect(ui.pb_get_sn_, &QPushButton::clicked, this, &QtDemo::HandleGetSN);

    connect(ui.combo_list_bound_mac_, SIGNAL(activated(int)), this, SLOT(HandleBoundMac(int)));
    connect(ui.pb_restart_, &QPushButton::clicked, this, &QtDemo::HandleRestart);

    connect(ui.pb_read_saved_mac_, &QPushButton::clicked, this, &QtDemo::HandleReadSavedMAC);
    connect(ui.pb_mac_unbinding_, &QPushButton::clicked, this, &QtDemo::HandleUnbinding);
    connect(ui.pb_read_local_mac_, &QPushButton::clicked, this, &QtDemo::HandleReadLocalMac);
    connect(ui.pb_binding_mac_, &QPushButton::clicked, this, &QtDemo::HandleBinding);

    connect(ui.pb_open_safe_door_alarm_, &QPushButton::clicked, this, &QtDemo::HandleOpenSafeDoorAlarm);
    connect(ui.pb_close_safe_door_alarm_, &QPushButton::clicked, this, &QtDemo::HandleCloseSafeDoorAlarm);
    connect(ui.pb_open_vibration_alarm_, &QPushButton::clicked, this, &QtDemo::HandleOpenVibrationAlarm);
    connect(ui.pb_close_vibration_alarm_, &QPushButton::clicked, this, &QtDemo::HandleCloseVibrationAlarm);

    connect(ui.pb_open_paper_door_, &QPushButton::clicked, this, &QtDemo::HandleOpenPaperDoor);
    connect(ui.pb_set_paper_timeout_, &QPushButton::clicked, this, &QtDemo::HandleSetPaperTimeout);

    connect(ui.pb_device_status_, &QPushButton::clicked, this, &QtDemo::HandleDeviceStatus);
    connect(ui.pb_door_status_, &QPushButton::clicked, this, &QtDemo::HandleDoorStatus);
    connect(ui.pb_open_safe_door_, &QPushButton::clicked, this, &QtDemo::HandleOpenSafeDoor);
    connect(ui.pb_close_safe_door_, &QPushButton::clicked, this, &QtDemo::HandleCloseSafeDoor);

    connect(ui.pb_open_led_, &QPushButton::clicked, this, &QtDemo::HandleOpenLED);
    connect(ui.pb_close_led_, &QPushButton::clicked, this, &QtDemo::HandleCloseLED);
    connect(ui.hori_sli_luminance_, SIGNAL(valueChanged(int)), this, SLOT(HandleSliderLuminance(int)));

    connect(ui.pb_beep_always_, &QPushButton::clicked, this, &QtDemo::HandleBeepAlways);
    connect(ui.pb_beep_interval_, &QPushButton::clicked, this, &QtDemo::HandleBeepInterval);
    connect(ui.pb_disable_beep_, &QPushButton::clicked, this, &QtDemo::HandleDisableBeep);

    connect(ui.pb_inside_test_mode_, &QPushButton::clicked, this, &QtDemo::HandleEnterTestMode);
    connect(ui.pb_outside_test_mode_, &QPushButton::clicked, this, &QtDemo::HandleExitTestMode);
    connect(ui.pb_move_x_, &QPushButton::clicked, this, &QtDemo::HandleMoveX);
    connect(ui.pb_move_y_, &QPushButton::clicked, this, &QtDemo::HandleMoveY);
    connect(ui.pb_turn_stamper_, &QPushButton::clicked, this, &QtDemo::HandleTurnStamper);

    connect(ui.pb_set_auth_code_, &QPushButton::clicked, this, &QtDemo::HandleSetAuthCode);
    connect(ui.pb_get_auth_code_, &QPushButton::clicked, this, &QtDemo::HandleGetAuthCode);

    connect(ui.pb_enable_debug_, &QPushButton::clicked, this, &QtDemo::HandleEnableDebug);
    connect(ui.pb_disable_debug_, &QPushButton::clicked, this, &QtDemo::HandleDisableDebug);

    connect(ui.pb_abc_check_, &QPushButton::clicked, this, &QtDemo::HandleABCCheck);

    connect(ui.btn_get_stamper_status_, &QPushButton::clicked, this, &QtDemo::HandleStamperStatus);

    //盖章页
    connect(ui.pb_capture_, &QPushButton::clicked, this, &QtDemo::HandleCapture);
    connect(ui.cb_stamp_ink_, SIGNAL(stateChanged(int)), this, SLOT(StampInk(int)));
    connect(ui.pb_pre_stamper_, &QPushButton::clicked, this, &QtDemo::HandlePreStamp);

    //升级页
    connect(ui.btn_select_updatefile_, &QPushButton::clicked, this, &QtDemo::HandleBtnSelectUpdateFile);
    connect(ui.btn_start_update_, &QPushButton::clicked, this, &QtDemo::HandleBtnStartUpdating);

    //EEPROM存储
    connect(ui.pb_read_capacity_version_, &QPushButton::clicked, this, &QtDemo::HandleBtnReadCapacityVersion);
    connect(ui.pb_write_data_, &QPushButton::clicked, this, &QtDemo::HandleWriteData);
    connect(ui.pb_read_data_, &QPushButton::clicked, this, &QtDemo::HandleReadData);
    connect(ui.pb_read_stamper_, &QPushButton::clicked, this, &QtDemo::HandleReadStampers);
    connect(ui.pb_load_stamper_mapping_, &QPushButton::clicked, this, &QtDemo::HandleLoadStamperMapping);
    connect(ui.pb_save_stamper_mapping_, &QPushButton::clicked, this, &QtDemo::HandleSaveStamperMapping);
    
    connect(ui.pb_write_conv_ratio_, &QPushButton::clicked, this, &QtDemo::HandleWriteConvRatio);
    connect(ui.pb_read_conv_ratio_, &QPushButton::clicked, this, &QtDemo::HandleReadConvRatio);

    connect(ui.pb_write_key_, &QPushButton::clicked, this, &QtDemo::HandleWriteKey);
    connect(ui.pb_read_key_, &QPushButton::clicked, this, &QtDemo::HandleReadKey);

    //RFID操作
    connect(ui.pb_rfid_tutorial_, &QPushButton::clicked, this, &QtDemo::HandleRfidTutorial);

    connect(ui.rb_stamper1_, SIGNAL(pressed()), this, SLOT(RfidStamper1()));
    connect(ui.rb_stamper2_, SIGNAL(pressed()), this, SLOT(RfidStamper2()));
    connect(ui.rb_stamper3_, SIGNAL(pressed()), this, SLOT(RfidStamper3()));
    connect(ui.rb_stamper4_, SIGNAL(pressed()), this, SLOT(RfidStamper4()));
    connect(ui.rb_stamper5_, SIGNAL(pressed()), this, SLOT(RfidStamper5()));
    connect(ui.rb_stamper6_, SIGNAL(pressed()), this, SLOT(RfidStamper6()));

    connect(ui.pb_all_stampers_, &QPushButton::clicked, this, &QtDemo::HandleAllaStamperStatus);
    connect(ui.pb_request_all_card_, &QPushButton::clicked, this, &QtDemo::HandleRequestAllCard);

    connect(ui.pb_select_stamper_, &QPushButton::clicked, this, &QtDemo::HandleSelectStamper);
    connect(ui.pb_get_rfid_, &QPushButton::clicked, this, &QtDemo::HandleGetRFID);
    connect(ui.combo_key_type_, SIGNAL(activated(int)), this, SLOT(HandleKeyType(int)));
    connect(ui.pb_set_block_addr_, &QPushButton::clicked, this, &QtDemo::HandleBtnSetBlockAddr);
    connect(ui.pb_key_verify_, &QPushButton::clicked, this, &QtDemo::HandleVerifyKey);
    connect(ui.pb_rfid_code_, &QPushButton::clicked, this, &QtDemo::HandleRFIDFactoryCode);
    connect(ui.pb_write_block_, &QPushButton::clicked, this, &QtDemo::HandleWriteBlock);
    connect(ui.pb_read_block_, &QPushButton::clicked, this, &QtDemo::HandleReadBlock);

    //盖章操作
    connect(ui.radio_stamper1_, SIGNAL(pressed()), this, SLOT(Stamper1()));
    connect(ui.radio_stamper2_, SIGNAL(pressed()), this, SLOT(Stamper2()));
    connect(ui.radio_stamper3_, SIGNAL(pressed()), this, SLOT(Stamper3()));
    connect(ui.radio_stamper4_, SIGNAL(pressed()), this, SLOT(Stamper4()));
    connect(ui.radio_stamper5_, SIGNAL(pressed()), this, SLOT(Stamper5()));
    connect(ui.radio_stamper6_, SIGNAL(pressed()), this, SLOT(Stamper6()));

    connect(ui.pb_enable_factory_, &QPushButton::clicked, this, &QtDemo::HandleEnableFactory);
    connect(ui.pb_disable_factory_, &QPushButton::clicked, this, &QtDemo::HandleDisableFactory);

    connect(ui.pb_stamp_read_stampers_, &QPushButton::clicked, this, &QtDemo::HandleStampReadStampers);
    connect(ui.cb_stamp_stamp_ink_, SIGNAL(stateChanged(int)), this, SLOT(HandleCheckStampInk(int)));

    connect(ui.hori_sli_x_, SIGNAL(valueChanged(int)), this, SLOT(HandleSliderX(int)));
    connect(ui.hori_sli_y_, SIGNAL(valueChanged(int)), this, SLOT(HandleSliderY(int)));
    connect(ui.hori_sli_angle_, SIGNAL(valueChanged(int)), this, SLOT(HandleSliderAngle(int)));

    connect(ui.pb_ordinary_stamp_, &QPushButton::clicked, this, &QtDemo::HandleOridinary);
    connect(ui.pb_sel_seal_, &QPushButton::clicked, this, &QtDemo::HandleSelSeal);
    connect(ui.pb_stamp_, &QPushButton::clicked, this, &QtDemo::HandleConfirmStamp);
    connect(ui.pb_check_mark_, &QPushButton::clicked, this, &QtDemo::HandleCheckMark);
    connect(ui.pb_cancel_stamp_, &QPushButton::clicked, this, &QtDemo::HandldeCancelStamp);
    connect(ui.pb_read_stamp_, &QPushButton::clicked, this, &QtDemo::HandleReadStamp);
    connect(ui.pb_read_abc_, &QPushButton::clicked, this, &QtDemo::HandleReadABC);
    connect(ui.pb_read_abc_index_, &QPushButton::clicked, this, &QtDemo::HandleReadABCIndex);

    //其他接口
    connect(ui.pb_lock_, &QPushButton::clicked, this, &QtDemo::HandleLock);
    connect(ui.pb_unlock_, &QPushButton::clicked, this, &QtDemo::HandleUnlock);
    connect(ui.pb_stamper_lock_, &QPushButton::clicked, this, &QtDemo::HandleIsLock);
    connect(ui.pb_write_id_, &QPushButton::clicked, this, &QtDemo::HandleWriteID);
    connect(ui.pb_read_id_, &QPushButton::clicked, this, &QtDemo::HandleReadID);
    connect(ui.pb_write_backup_sn_, &QPushButton::clicked, this, &QtDemo::HandleWriteBackupSN);
    connect(ui.pb_read_backup_sn_, &QPushButton::clicked, this, &QtDemo::HandleReadBackupSN);
    connect(ui.pb_set_sidedoor_, &QPushButton::clicked, this, &QtDemo::HandleSetSideDoor);

    connect(ui.pb_write_mac_, &QPushButton::clicked, this, &QtDemo::HandleWriteMAC);
    connect(ui.pb_read_mac_, &QPushButton::clicked, this, &QtDemo::HandleReadMAC);

    connect(ui.pb_read_voltage_, &QPushButton::clicked, this, &QtDemo::HandleReadVoltage);

    connect(ui.pb_write_cal_points_, &QPushButton::clicked, this, &QtDemo::HandldeWriteCalPoints);
    connect(ui.pb_read_cal_points_, &QPushButton::clicked, this, &QtDemo::HandleReadPCalPoints);

    connect(ui.pb_read_range_, &QPushButton::clicked, this, &QtDemo::HandleReadRange);

    connect(ui.pb_read_alarm_, &QPushButton::clicked, this, &QtDemo::HandleReadAlarm);

    connect(ui.pb_hardware_ver_, &QPushButton::clicked, this, &QtDemo::HandleHardwareVer);
    
    //默认显示第一个tab
    ui.tabWidget->removeTab(2);  //隐藏升级tab
    ui.tabWidget->setCurrentIndex(0);

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
//     case 2:
//         InitUpdate();
//         break;
    case 3:
        InitRfidPage();
        break;
    case 4:
        InitStamp();
        break;
    case 5:
        InitOther();
        break;
    default:
        break;
    }
}

bool QtDemo::Init()
{
    /* init boost log
    * 1. Add common attributes
    * 2. set log filter to trace
    */
    boost::log::add_common_attributes();
    boost::log::core::get()->add_global_attribute(
        "Scope",
        boost::log::attributes::named_scope());
    boost::log::core::get()->set_filter(
        boost::log::trivial::severity >= boost::log::trivial::trace);

    /* log formatter:
    * [TimeStamp] [ThreadId] [Severity Level] [Scope] Log message
    */
    auto fmtTimeStamp = boost::log::expressions::
        format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
    auto fmtThreadId = boost::log::expressions::
        attr<boost::log::attributes::current_thread_id::value_type>("ThreadID");
    auto fmtSeverity = boost::log::expressions::
        attr<boost::log::trivial::severity_level>("Severity");
    auto fmtScope = boost::log::expressions::format_named_scope("Scope",
        boost::log::keywords::format = "%n(%f:%l)",
        boost::log::keywords::iteration = boost::log::expressions::reverse,
        boost::log::keywords::depth = 2);
    boost::log::formatter logFmt =
        boost::log::expressions::format("[%1%] (%2%) [%3%] [%4%] %5%")
        % fmtTimeStamp % fmtThreadId % fmtSeverity % fmtScope
        % boost::log::expressions::smessage;

    /* console sink */
    auto consoleSink = boost::log::add_console_log(std::clog);
    consoleSink->set_formatter(logFmt);

    /* fs sink */
    std::string prefix;
    std::string log_file = "封闭式印控机V2.4_%Y-%m-%d_%H-%M-%S.%N.log";
    bool err = base::FSUtility::GetMoudulePath(prefix);
    auto fsSink = boost::log::add_file_log(
        boost::log::keywords::file_name = err? prefix + log_file: log_file,
        boost::log::keywords::rotation_size = 10 * 1024 * 1024,
        boost::log::keywords::min_free_space = 30 * 1024 * 1024,
        boost::log::keywords::open_mode = std::ios_base::app);
    fsSink->set_formatter(logFmt);
    fsSink->locked_backend()->auto_flush(true);

    echo_thread_ = new boost::thread(boost::bind(&QtDemo::EventWorker, this));
    StartTimer(1000);

    InitDevControl(); //需要手动调用一次
    return true;
}

QtDemo::~QtDemo()
{
    if (NULL != update_thread_) {
        update_thread_->join();
        delete update_thread_;
    }
}

/////////////////////////// 主界面 ////////////////////////////////

//打开设备
void QtDemo::HandleBtnOpenDev()
{
    if (open_called) {
        Info(cmd_des[OC_DEV_ALREADY_OPENED]);
        return;
    }

    BOOST_LOG_FUNCTION();
    BOOST_LOG_TRIVIAL(info) << "打开设备";

    OpenEventS* open = new OpenEventS(this);
    event_queue_.Push(open);
}

//关闭设备
void QtDemo::HandleBtnCloseDev()
{
    CloseEvent* close = new CloseEvent(this);
    event_queue_.Push(close);
}

/////////////////////////// 设备控制 ////////////////////////////////

void QtDemo::InitDevControl()
{
    if (register_conn_cb_ != 0)
        BOOST_LOG_TRIVIAL(error) << "注册设备回掉函数失败";
    if (register_msg_cb_ != 0)
        BOOST_LOG_TRIVIAL(error) << "注册回掉函数(盖章结果)失败";

    ui.hori_sli_luminance_->setMinimum(1);
    ui.hori_sli_luminance_->setMaximum(100);
    ui.hori_sli_luminance_->setValue(DEFAULT_LUMINANCE);
    ui.te_led_luminance_->setText(QString::number(DEFAULT_LUMINANCE));

    ui.combo_led_ctrl_->clear();
    ui.combo_led_ctrl_->addItem(QString::fromLocal8Bit("安全门旁补光灯"));
    ui.combo_led_ctrl_->addItem(QString::fromLocal8Bit("凭证摄像头旁补光灯"));
    ui.combo_led_ctrl_->setCurrentIndex(0);

    ui.le_paper_timeout_->setText("30");

    ui.le_stamper_idx_->setText(QString::number(1)); //从1开始
}

void QtDemo::HandleSystemInfo()
{
    char info[3] = { 0 };
    int ret = FGetSystemMsg(info, sizeof(info));
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取系统信息失败"));

    char* system[] =
    {
        "主系统",
        "备用系统",
        "主系统升级模式",
        "备用系统升级模式"
    };

    char msg[512] = { 0 };
    sprintf_s(msg, "%s,\n硬件系列号:%d,\n硬件版本号:%d", system[info[0]], info[1], info[2]);
    Info(QString::fromLocal8Bit(msg));
}

void QtDemo::HandleFirewareVersion()
{
    char version[9] = { 0 };
    int ret = ::FGetFirwareVer(version, 8);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取固件版本号失败"));

    unsigned short year = 0;
    memcpy(&year, version, sizeof(unsigned short));

    char month = version[2] * 10 + version[3];
    char day = version[4] * 10 + version[5];
    char ver = version[6] * 10 + version[7];

    Info(
        QString::fromLocal8Bit("固件版本号:\n") +
        QString::number(2010 + year) + QString::fromLocal8Bit("年") +
        QString::number(month) + QString::fromLocal8Bit("月") + 
        QString::number(day) + QString::fromLocal8Bit("日") +
        QString::number(ver) + QString::fromLocal8Bit("版"));
}

void QtDemo::HandleExitEntainmainMode()
{
    int ret = ::FQuitMaintainMode();
    if (ret != STF_SUCCESS)
        return Info(QString::fromLocal8Bit("退出维护模式失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("退出维护模式成功"), STATUS_TEXT);
}

void QtDemo::HandleEnterEntainmainMode()
{
    int ret = ::FMaintenanceMode();
    if (ret != STF_SUCCESS)
        return Info(QString::fromLocal8Bit("进入维护模式失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("进入维护模式成功"), STATUS_TEXT);
}

void QtDemo::HandleSetSN()
{
    std::string sn = ui.te_show_sn_->toPlainText().toStdString();
    if (sn.empty())
        return Info(QString::fromLocal8Bit("序列号为空"));

    int ret = FSetFirwareSN((char*)sn.c_str(), sn.length());
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("设置序列号失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置序列号成功"), STATUS_TEXT);
}

void QtDemo::HandleGetSN()
{
    char sn[512] = { 0 };
    int ret = FGetFirwareSN(sn, 15);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取序列号失败"));

    ui.te_show_sn_->setPlainText(QString::fromStdString(sn));
    ui.statusBar->showMessage(QString::fromLocal8Bit("获取序列号成功"), STATUS_TEXT);
}

void QtDemo::HandleReset()
{
    int ret = ::Reset();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("复位失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("复位成功"), STATUS_TEXT);
}

void QtDemo::HandleRestart()
{
    int ret = ::FRestart();
    if (ret != STF_SUCCESS)
        return Info(QString::fromLocal8Bit("重启主板失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("重启主板成功"), STATUS_TEXT);
}

void QtDemo::HandleBoundMac(int index)
{
    macs_.combo_idx_ = ui.combo_list_bound_mac_->currentIndex();
    std::string mac = ui.combo_list_bound_mac_->currentText().toStdString();
    std::map<std::string, int>::iterator it = macs_.bound_mac_.find(mac);
    if (it != macs_.bound_mac_.end()) {
        macs_.unbind_mac_ = mac;
        macs_.unbind_idx_ = it->second;
    }
}

void QtDemo::HandleReadSavedMAC()
{
    if (!IsOpened())
        return;

    unsigned char mac1[64] = { 0 };
    unsigned char mac2[64] = { 0 };
    int ret = ReadMAC(mac1, mac2, 17);
    if (ret != STF_SUCCESS)
        return Info(QString::fromLocal8Bit("读取MAC地址失败"));

    ui.combo_list_bound_mac_->clear(); //成功清空原combox

    if (strlen((char*)mac1) != 0) {
        ui.combo_list_bound_mac_->addItem(QString::fromStdString((char*)mac1));
        macs_.bound_mac_.insert(std::make_pair((char*)mac1, 1));
    }

    if (strlen((char*)mac2) != 0) {
        ui.combo_list_bound_mac_->addItem(QString::fromStdString((char*)mac2));
        macs_.bound_mac_.insert(std::make_pair((char*)mac2, 2));
    }

    ui.combo_list_bound_mac_->adjustSize();
}

void QtDemo::HandleUnbinding()
{
    if (macs_.unbind_idx_ < 0)
        return Info(QString::fromLocal8Bit("指定待解绑MAC地址"));

    int ret = -1;
    unsigned char unbind_str[17] = { 0 };
    if (1 == macs_.unbind_idx_)
        ret = WriteMAC(unbind_str, NULL);
    else
        ret = WriteMAC(NULL, unbind_str);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("解绑MAC地址失败"));

    std::map<std::string, int>::iterator it = macs_.bound_mac_.find(macs_.unbind_mac_);
    macs_.bound_mac_.erase(it);
    ui.combo_list_bound_mac_->removeItem(macs_.combo_idx_);

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功解绑MAC地址"), STATUS_TEXT);
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
    if (2 <= macs_.bound_mac_.size())
        return Info(QString::fromLocal8Bit("已绑定2个地址, 请先解绑一个MAC"));

    std::string mac = ui.le_mac_->text().toStdString();
    if (mac.empty())
        return Info(QString::fromLocal8Bit("待绑定MAC地址为空"));

    std::map<std::string, int>::iterator it = macs_.bound_mac_.find(mac);
    if (it != macs_.bound_mac_.end())
        return Info(QString::fromLocal8Bit("待绑定MAC地址已在绑定列表中"));

    int bind_idx = 0;
    if (macs_.bound_mac_.empty())
        bind_idx = 1; //1或者2
    else
        bind_idx = macs_.bound_mac_.begin()->second == 1 ? 2 : 1;

    int ret = -1;
    if (1 == bind_idx)
        ret = WriteMAC((unsigned char*)mac.c_str(), NULL, mac.length());
    else
        ret = WriteMAC(NULL, (unsigned char*)mac.c_str(), 0, mac.length());

    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("绑定第") + 
        QString::number(bind_idx) + QString::fromLocal8Bit("个MAC地址失败"));

    macs_.bound_mac_.insert(std::make_pair(mac, bind_idx));
    ui.combo_list_bound_mac_->addItem(QString::fromStdString(mac));
    ui.combo_list_bound_mac_->adjustSize();

    ui.statusBar->showMessage(QString::fromLocal8Bit("绑定MAC地址成功"), STATUS_TEXT);
}

void QtDemo::HandleOpenSafeDoorAlarm()
{
    int ret = ::SetAlarm(0, 1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开安全门报警失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功打开安全门报警"), STATUS_TEXT);
}

void QtDemo::HandleCloseSafeDoorAlarm()
{
    int ret = ::SetAlarm(0, 0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭安全门报警失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功关闭安全门报警"), STATUS_TEXT);
}

void QtDemo::HandleOpenVibrationAlarm()
{
    int ret = ::SetAlarm(1, 1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开振动报警失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功打开振动报警"), STATUS_TEXT);
}

void QtDemo::HandleCloseVibrationAlarm()
{
    int ret = ::SetAlarm(1, 0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭振动报警失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功关闭振动报警"), STATUS_TEXT);
}

void QtDemo::HandleSetPaperTimeout()
{
    std::string str = ui.le_paper_timeout_->text().toStdString();
    int ret = SetPaperDoor(atoi(str.c_str()));
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("设置进纸门超时提示失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功设置进纸门超时提示"), STATUS_TEXT);
}

void QtDemo::HandleOpenPaperDoor()
{
    int ret = ::FOpenDoorPaper();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开进纸门失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功打开进纸门"), STATUS_TEXT);
}

void QtDemo::HandleOpenSafeDoor()
{
    int ret = FOpenDoorSafe();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开安全门失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("打开安全门成功"), STATUS_TEXT);
}

void QtDemo::HandleCloseSafeDoor()
{
    int ret = FCloseDoorSafe();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭安全门失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关闭安全门成功"), STATUS_TEXT);
}

void QtDemo::HandleDeviceStatus()
{
    unsigned char status[15] = { 0 };
    int ret = ::FGetDevStatus(status, 14);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("查询设备状态失败"));

    char* status_des[] =
    {
        "未初始化",
        "启动自检",
        "检测章",
        "空闲状态",
        "测试模式",
        "故障模式",
        "盖章模式",
        "维护模式"
    };
    char des[256] = { 0 };
    sprintf_s(des, "设备处于:%s", status_des[status[1]]);
    Info(QString::fromLocal8Bit(des));
    ui.statusBar->showMessage(QString::fromLocal8Bit("查询设备状态成功"), STATUS_TEXT);
}

void QtDemo::HandleDoorStatus()
{
    char door[4] = { 0 };
    int ret = ::FGetDoorsPresent(door, 4);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取门状态失败"));

    char* info[] =
    {
        "关闭",
        "开启",
        "检测错误"
    };
    char msg[512] = { 0 };
    sprintf_s(msg,
        "推纸门:%s\n"
        "电子锁:%s\n"
        "机械锁:%s\n"
        "顶盖门:%s", info[door[0]], info[door[1]], info[door[2]], info[door[3]]);
    Info(QString::fromLocal8Bit(msg));
}

void QtDemo::HandleOpenLED()
{
    int ret = FLightCtrl(ui.combo_led_ctrl_->currentIndex() + 1, 1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开补光灯失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("打开补光灯成功"), STATUS_TEXT);
}

void QtDemo::HandleCloseLED()
{
    int ret = FLightCtrl(ui.combo_led_ctrl_->currentIndex() + 1, 0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭补光灯失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关闭补光灯成功"), STATUS_TEXT);
}

void QtDemo::HandleSliderLuminance(int val)
{
    if (!open_called)
        return;

    int luminance = ui.hori_sli_luminance_->value();
    int ret = ::FLightBrightness(ui.combo_led_ctrl_->currentIndex() + 1, luminance);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("补光灯亮度调节失败"));

    ui.te_led_luminance_->setPlainText(QString::number(luminance));
    ui.statusBar->showMessage(QString::fromLocal8Bit("补光灯亮度调节成功"), STATUS_TEXT);
}

void QtDemo::HandleEnterTestMode()
{
    if (test_mode_)
        return Info(QString::fromLocal8Bit("设备已经处于测试模式"));

    int ret = ::FInTestMode();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("进入测试模式失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功进入测试模式"), STATUS_TEXT);
    test_mode_ = true;
}

void QtDemo::HandleExitTestMode()
{
    if (!test_mode_)
        return Info(QString::fromLocal8Bit("设备未处于测试模式"));

    int ret = ::FOutTestMode();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("退出测试模式失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功退出测试模式"), STATUS_TEXT);
    test_mode_ = false;
}

void QtDemo::HandleMoveX()
{
    if (!test_mode_)
        return Info(QString::fromLocal8Bit("设备处于测试模式下才能单独控制X轴"));

    std::string x = ui.te_move_x_->toPlainText().toStdString();
    int ret = ::FMoveX(atoi(x.c_str()));
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("移动X轴失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("移动X轴成功"), STATUS_TEXT);
}

void QtDemo::HandleMoveY()
{
    if (!test_mode_)
        return Info(QString::fromLocal8Bit("设备处于测试模式下才能单独控制Y轴"));

    std::string y = ui.te_move_y_->toPlainText().toStdString();
    int ret = ::FMoveY(atoi(y.c_str()));
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("移动Y轴失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("移动Y轴成功"), STATUS_TEXT);
}

void QtDemo::HandleTurnStamper()
{
    std::string angle = ui.te_turn_stamper_->toPlainText().toStdString();
    int ret = ::FTurnSeal(atoi(angle.c_str()));
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("转章失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("转章成功"), STATUS_TEXT);
}

void QtDemo::HandleBeepAlways()
{
    int ret = ::FBeepCtrl(1, 0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("设置蜂鸣器长鸣失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置蜂鸣器长鸣成功"), STATUS_TEXT);
}

void QtDemo::HandleBeepInterval()
{
    std::string interval = ui.te_beep_interval_->toPlainText().toStdString();
    int ret = ::FBeepCtrl(2, atoi(interval.c_str()));
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("设置蜂鸣器间隔响失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置蜂鸣器间隔响成功"), STATUS_TEXT);
}

void QtDemo::HandleDisableBeep()
{
    int ret = ::FBeepCtrl(0, 0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("设置蜂鸣器长鸣失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置蜂鸣器长鸣成功"), STATUS_TEXT);
}

void QtDemo::HandleInfraredStatus()
{
    if (!IsOpened())
        return;

    char infrared[128] = { 0 };
    int ret = FGetInfraRedStatus(infrared, sizeof(infrared));
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取红外状态失败"));

    if (infrared[0] == 0)
        return Info(QString::fromLocal8Bit("电机忙"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("获取红外成功"), STATUS_TEXT);
}

void QtDemo::HandleABCCheck()
{
    std::string str = ui.le_stamper_idx_->text().toStdString();
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
    BOOST_LOG_TRIVIAL(info) << ret;
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("农行校准失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("农行校准成功"), STATUS_TEXT);
}

void QtDemo::HandleStamperStatus()
{
    if (!IsOpened())
        return;

    char status[31] = { 0 };
    int ret = ::FGetSealPresent(status, 30);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取印章状态失败"));

    QLabel* labels[] = 
    {
        ui.label_stamper1_des_,
        ui.label_stamper2_des_,
        ui.label_stamper3_des_,
        ui.label_stamper4_des_,
        ui.label_stamper5_des_,
        ui.label_stamper6_des_
    };

    if (status[0] == 0x00) {
        for (int i = 0; i < 6; ++i)
            labels[i]->setText(QString::fromLocal8Bit("无章"));
    } else if (status[0] >= 1 && status[0] <= 6) {
        for (int i = 0; i < 6; ++i) {
            unsigned int stamper_id = 0;
            memcpy(&stamper_id, &status[4 * i + 1], 4);
            if (0 == stamper_id)
                labels[i]->setText(QString::fromLocal8Bit("无章"));
            else
                labels[i]->setText(QString::fromLocal8Bit("有章"));
            labels[i]->adjustSize();
        }
    } else {
        Info(QString::fromLocal8Bit("无效章个数"));
    }

    //机械手臂
    unsigned char mechanical = status[25];
    char stamper_armed[256] = { 0 };
    if (mechanical == 0xFF)
        sprintf_s(stamper_armed, "%s", "无章");
    else if (mechanical >= 0 && mechanical < 6)
        sprintf_s(stamper_armed, "%d号章", mechanical + 1);
    else
        return Info(QString::fromLocal8Bit("接收消息格式错误"));

    char label[512] = { 0 };
    sprintf_s(label, "机械手臂上:%s", stamper_armed);
    ui.label_mechanical_arm_->setText(QString::fromStdString(label));
    ui.label_mechanical_arm_->adjustSize();
    ui.statusBar->showMessage(QString::fromLocal8Bit("获取印章状态成功"), STATUS_TEXT);
}

void QtDemo::HandleSetAuthCode()
{
    std::string code = ui.le_auth_code_->text().toStdString();
    int ret = SetDevCode((char*)code.c_str(), code.length());
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("设置设备认证码失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置设备认证码成功"), STATUS_TEXT);
}

void QtDemo::HandleGetAuthCode()
{
    char code[512] = { 0 };
    int ret = GetDevCode(code, sizeof(code));
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取设备认证码失败"));

    ui.le_auth_code_->setText(QString::fromStdString(code));
    ui.statusBar->showMessage(QString::fromLocal8Bit("获取设备认证码成功"), STATUS_TEXT);
}

void QtDemo::HandleEnableDebug()
{
    int ret = FDebugLogSwitch(1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("开启debug失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功开启debug"), STATUS_TEXT);
}

void QtDemo::HandleDisableDebug()
{
    int ret = FDebugLogSwitch(0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭debug失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功关闭debug"), STATUS_TEXT);
}

/////////////////////////////// 盖章 /////////////////////////////////////

void QtDemo::StampInk(int checked)
{
    bool checkeds = ui.cb_stamp_ink_->isChecked();
    char stamper = 1;
    std::srand(std::time(0));
    int ret = ::FSelectStamper(std::rand(), stamper, checkeds? 1: 0);
    char msg[512] = { 0 };
    sprintf_s(msg, "选取印章:%d, %s", stamper, checkeds ? "蘸印油" : "不蘸印油");
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit(msg));

    ui.statusBar->showMessage(QString::fromLocal8Bit(msg), STATUS_TEXT);
}

void QtDemo::HandleCapture()
{
    img_->load("d:\\Users\\Kailani\\Downloads\\USBControlF60_2.4\\bin\\rfid.png");
    ui.label_image_->setPixmap(QPixmap::fromImage(*img_));
    ui.label_image_->show();
}

void QtDemo::HandlePreStamp()
{

}

void QtDemo::InitSnapshot()
{
    if (img_ != NULL)
        return;

    img_ = new QImage();
}

/////////////////////////////// 升级 ////////////////////////////////////

HANDLE QtDemo::UpdateEvent;

void QtDemo::InitUpdate()
{
    ui.pb_display_progress_->setRange(0, 1000);
    ui.pb_display_progress_->setValue(0);

    UpdateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

void QtDemo::HandleBtnSelectUpdateFile()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        QString::fromLocal8Bit("选择文件"),
        "",
        tr("fbin (*.fbin"));
    if (filename.isEmpty())
        return Info(QString::fromLocal8Bit("未选择任何文件"));
    else if (!filename.contains(".fbin"))
        return Info(QString::fromLocal8Bit("选择的不是升级文件"));
    else;

    bin_file_ = filename.toStdString();
    ui.le_display_path_->setText(filename);
}

void QtDemo::HandleBtnStartUpdating()
{
    ResetEvent(UpdateEvent);
    update_thread_ = new (std::nothrow) boost::thread(
        boost::bind(&QtDemo::firewareUpdateThread, this));
}

void QtDemo::firewareUpdateThread()
{
    int ret = 0;
    //已读文件数据大小
    unsigned long filereadsize = 0;
    // 文件长度
    unsigned long filesize = 0;
    //文件头
    int headlen = 512;

    //读取文件
    //设置程序使用本地化信息
    std::locale::global(std::locale(""));
    std::fstream fin(bin_file_.c_str(), std::fstream::binary | std::fstream::in);
    if (!fin) {//文件打开失败
        ret = -1;
        return;
    }

    fin.seekg(0, std::fstream::end);
    filesize = (unsigned long)fin.tellg();
    fin.seekg(0, std::fstream::beg);

    //获取版本信息
    char strv[3] = { 0 };
    char szCmp[2] = { 0 };
    char szFCmp[2] = { 0 };
    ret = ::FGetSystemMsg(strv, sizeof(strv));
    if (ret == 0) {
        szCmp[0] = strv[1];
        szCmp[1] = strv[2];
    } else {
        //获取设备信息失败
        return;
    }

    unsigned char ReadBinBuffer[512] = { 0 };
    fin.read((char*)ReadBinBuffer, sizeof(ReadBinBuffer));
    filereadsize = (unsigned long)fin.gcount();
    szFCmp[0] = ReadBinBuffer[14];
    szFCmp[1] = ReadBinBuffer[16];

    if (szCmp[0] != szFCmp[0] || szFCmp[1] != szCmp[1]) {
        //版本不一致
        ret = -1;
        return;
    }

    //比较版本，如果版本不正确，则不进行升级
    ret = BinFileCheck(ReadBinBuffer, filereadsize);
    if (ret != 0)
        goto _ThreadErrorRet;

    int sys = 0;
    ret = Syscheck(ReadBinBuffer[2], sys);
    if (ret != 0)
        goto _ThreadErrorRet;

    int retB5_B1 = -1;
    retB5_B1 = MCUcheck_SendB1(ReadBinBuffer, 32);
    if (retB5_B1 != 0)
    {
        retB5_B1 = ret;
        goto _ThreadErrorRet;
    }

    if (sys < 2)
    {
        //等待重连
        int dwRet = 0;
        dwRet = ::WaitForSingleObject(UpdateEvent, 20000);
        if (dwRet != WAIT_OBJECT_0)
        {
            ret = -6;
            goto _ThreadErrorRet;
        }

        //再次验证
        retB5_B1 = MCUcheck_SendB1(ReadBinBuffer, 32);
        if (retB5_B1 != 0)
        {
            ret = retB5_B1;
            goto _ThreadErrorRet;
        }
    }

    //发送0xB2, 0xB3, 0xB4，完成文件有效数据发送

    int sendret = 0;
    int readlen = 59;
    //减去文件头，filesize 为文件的有效数据
    filesize -= headlen;
    //已读数据大小
    int readResult = 0;

    memset(ReadBinBuffer, 0, sizeof(ReadBinBuffer));
    fin.read((char*)ReadBinBuffer, readlen);

    sendret = (int)::FfirewareUpdate(0xB2, ReadBinBuffer, (int)fin.gcount());
    if (sendret != 0) {
        ret = -7;
        goto _ThreadErrorRet;
    }

    readResult += (int)fin.gcount();

    while (fin) {
        memset(ReadBinBuffer, 0, readlen);
        fin.read((char*)ReadBinBuffer, readlen);
        readResult += (int)fin.gcount();;

        if (filesize - readResult > 0)
        {
            sendret = ::FfirewareUpdate(0xB3, ReadBinBuffer, (int)fin.gcount());
        }
        else
        {
            sendret = ::FfirewareUpdate(0xB4, ReadBinBuffer, (int)fin.gcount());
        }

        if (sendret != 0)
        {
            ret = -7;
            goto _ThreadErrorRet;
        }

        ui.pb_display_progress_->setValue(readResult * 1000 / filesize);
    }

_ThreadErrorRet:
    std::string strUpdate;
    switch (ret)
    {
    case 0:
        strUpdate = "成功";
        break;
    case -1:
        strUpdate = "文件打开失败";
    case -2:
        strUpdate = "文件头校验失败";//
    case -3:
        strUpdate = "系统不匹配";////
    case -4:
        strUpdate = "MCU校验失败";////
    case -5:
        strUpdate = "通知升级失败";////通知升级失败
    case -6:
        strUpdate = "等待重连超时";////等待重连超时
    case -7:
        strUpdate = "数据发送出错";////数据发送出错
    default:
        break;
    }

    if (fin)
    {
        fin.close();
    }
}

int QtDemo::BinFileCheck(const unsigned char* pReadBinBuffer, int len)
{
    int ret = 0;
    int lenOR = 509;

    if (pReadBinBuffer[0] != 0x55 || pReadBinBuffer[1] != 0x5A)
    {//文件头错误
        ret = -2;
        goto _ErrorRet;
    }

    unsigned char XorVal = 0x00;
    for (int i = 0; i < lenOR; i++)
    {
        XorVal ^= pReadBinBuffer[i];
    }

    if (pReadBinBuffer[lenOR] != XorVal)
    {//校验出错
        ret = -2;
        goto _ErrorRet;
    }

    if (pReadBinBuffer[510] != 0xAA || pReadBinBuffer[511] != 0xA5)
    {//文件尾错误
        ret = -2;
        goto _ErrorRet;
    }

_ErrorRet:
    return ret;
}

//系统检查
int QtDemo::Syscheck(unsigned char fileSys, int &sys)
{
    //查询系统
    char strsys[3] = { 0 };
    int ret = -1;
    sys = 0;
    ret = ::FGetSystemMsg(strsys, sizeof(strsys));
    sys = strsys[0];
    if (fileSys == 0x01)
    {//主系统升级文件
        if (sys == 0x00 || sys == 0x02)
        {//主系统或者主系统升级模式
            ret = 0;
        }
    }

    if (fileSys == 0x02)
    {//备用系统升级文件
        if (sys == 0x01 || sys == 0x03)
        {//备用系统或者主系统升级模式
            ret = 0;
        }
    }

    return ret;
}

//发送0xB5命令，进行MUC验证
int QtDemo::MCUcheck_SendB1(unsigned char* pReadBinBuffer, int len)
{
    int retB5 = -1;
    retB5 = ::FfirewareUpdate(0xB5, pReadBinBuffer, 32);
    if (retB5 != 0)
    {
        return -4;
    }

    int retB1 = -1;
    retB1 = ::FfirewareUpdate(0xB1, NULL, 0);
    if (retB1 != 0)
    {
        return -4;
    }

    return 0;
}

////////////////////////// EEPROM存储 ////////////////////////////////////

//读大小及版本号
void QtDemo::HandleBtnReadCapacityVersion()
{
    unsigned char version = 0;
    unsigned short mem = 0;
    int ret = GetStorageCapacity(version, mem);
    if (STF_SUCCESS != ret)
        return Info(cmd_des[OC_READ_CAPA_VERSION_FAIL]);

    ui.le_capacity_->setText(QString::number(mem));
    ui.le_version_->setText(QString::number(version));
}

//写EEPROM数据
void QtDemo::HandleWriteData()
{
    if (!IsOpened())
        return Info(QString::fromLocal8Bit("设备未打开"));

    if (ui.le_write_offset_->text().isEmpty())
        Info(QString::fromLocal8Bit("指定写偏移量"));

    unsigned short offset = atoi((const char*)ui.le_write_offset_->text().toStdString().c_str());
    std::string data_to_be_write = ui.te_write_data_->toPlainText().toStdString();

    int ret = WriteIntoStamper(offset, (unsigned char*)data_to_be_write.c_str(), data_to_be_write.length());
    Info(ret == STF_SUCCESS ? cmd_des[OC_WRITE_DATA_SUC]: cmd_des[OC_WRITE_DATA_FAIL]);
}

//从EEPROM读数据
void QtDemo::HandleReadData()
{
    if (!IsOpened())
        return Info(QString::fromLocal8Bit("设备未打开"));

    std::string _off = ui.le_read_offset_->text().toStdString();
    if (_off.empty())
        return Info(QString::fromLocal8Bit("指定读起始位置"));

    unsigned short offset = atoi(_off.c_str());
    unsigned short len = atoi((const char*)ui.le_read_len_->text().toLocal8Bit());

    unsigned char ret_len = 0;
    unsigned char recv_data[512] = { 0 };
    int ret = ReadStamperMem(offset, len, recv_data, ret_len);
    if (STF_SUCCESS != ret)
        return Info(cmd_des[OC_READ_DATA_FAIL]);

    QString s = QString::fromStdString((char*)recv_data);
    ui.te_read_data_->setPlainText(s);
    ui.statusBar->showMessage(cmd_des[OC_READ_DATA_SUC], STATUS_TEXT);
}

void QtDemo::EventWorker()
{
    while (true) {
        Event* evt = NULL;
        if (0 == event_queue_.WaitForRead(INFINITE)) {
            if (event_queue_.Pop(evt))
                evt->Execute();
        }
    }
}

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

//读取章状态
void QtDemo::HandleReadStampers()
{
    if (!IsOpened())
        return;

    BOOST_LOG_FUNCTION();
    unsigned int rfids[6] = { 0 };
    for (int i = 0; i < 6; ++i) {
        int err = GetStamperID(i, rfids[i]);
        if (STF_SUCCESS == err) {
            char text[20] = { 0 };
            sprintf_s(text, "0x%x", rfids[i]);
            current_ids[i] = QString::fromStdString(text);
            continue;
        }
        
        BOOST_LOG_TRIVIAL(error) << "获取印章仓位号是" << i << "的RFID失败." << "err:" << err;
    }

    QLabel* label_stampers[] = {
        ui.label_stamper1_rfid_,
        ui.label_stamper2_rfid_,
        ui.label_stamper3_rfid_,
        ui.label_stamper4_rfid_,
        ui.label_stamper5_rfid_,
        ui.label_stamper6_rfid_};

    for (int i = 0; i < 6; ++i) {
        char text[20] = { 0 };
        sprintf_s(text, "0x%x", rfids[i]);
        if (label_stampers[i] != NULL) {
            label_stampers[i]->setText(QString::fromLocal8Bit(text));
            label_stampers[i]->adjustSize();
        }
    }
}

//加载章映射
void QtDemo::HandleLoadStamperMapping()
{
    if (!IsOpened())
        return;

    BOOST_LOG_FUNCTION();
    char mapped_info[48] = { 0 };
    int ret = GetStampMap(mapped_info, 48);
    if (STF_SUCCESS != ret) {
        BOOST_LOG_TRIVIAL(error) << "获取章映射失败, err:" << ret;
        return Info(QString::fromLocal8Bit("获取章映射失败"));
    }

    unsigned int rfids[6] = { 0 };
    for (int i = 0; i < 6; ++i) {
        memcpy(&rfids[i], mapped_info + i * 8, sizeof(unsigned int));
    }

    QLabel* label_stampers[] = {
        ui.label_stamper1_rfid_,
        ui.label_stamper2_rfid_,
        ui.label_stamper3_rfid_,
        ui.label_stamper4_rfid_,
        ui.label_stamper5_rfid_,
        ui.label_stamper6_rfid_ };
    for (int i = 0; i < 6; ++i) {
        char text[128] = { 0 };
        sprintf_s(text, "0x%x", rfids[i]);
        label_stampers[i]->setText(current_ids[i] + " " + QString::fromStdString(text));
        label_stampers[i]->adjustSize();
    }
}

//保存章映射
void QtDemo::HandleSaveStamperMapping()
{
    int ret = SetStampMap();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("设置章映射失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置章映射成功"), STATUS_TEXT);
}

void QtDemo::HandleWriteConvRatio()
{
    float x, y;
    x = atof(ui.le_conv_ratio_x_->text().toStdString().c_str());
    y = atof(ui.le_conv_ratio_y_->text().toStdString().c_str());
    int ret = ::WriteImageConvRatio(&x, &y);
    if (ret != STF_SUCCESS)
        return Info(QString::fromLocal8Bit("存储图像转换倍率失败"));
    
    ui.statusBar->showMessage(QString::fromLocal8Bit("存储图像转换倍率成功"), STATUS_TEXT);
}

void QtDemo::HandleReadConvRatio()
{
    float x, y;
    int ret = ::ReadImageConvRatio(&x, &y);
    if (ret != STF_SUCCESS)
        return Info(QString::fromLocal8Bit("读取图像转换倍率失败"));

    char buf[10] = { 0 };
    sprintf_s(buf, "%.3f", x);
    ui.le_conv_ratio_x_->setText(buf);

    sprintf_s(buf, "%.3f", y);
    ui.le_conv_ratio_y_->setText(buf);

    ui.statusBar->showMessage(QString::fromLocal8Bit("读取图像转换倍率成功"), STATUS_TEXT);
}

void QtDemo::HandleWriteKey()
{
    std::string key = ui.le_write_key_->text().toStdString();

    int ret = WriteKey(
        key.empty() ? NULL : (unsigned char*)key.c_str(),
        key.length());

    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("写key值失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("写key值成功"), STATUS_TEXT);
}

void QtDemo::HandleReadKey()
{
    unsigned char key[17] = { 0 };
    int ret = ReadKey(
        key, 
        17);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("读key值失败"));

    ui.le_read_key_->setText(QString::fromStdString((char*)key));
    ui.statusBar->showMessage(QString::fromLocal8Bit("读key值成功"), STATUS_TEXT);
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
    ui.le_wait_time_->setText(QString("%1").arg(StampPara::DEFAULT_WAIT));

    ui.hori_sli_x_->setMinimum(0);
    ui.hori_sli_x_->setMaximum(StampPara::MAX_X);
    ui.hori_sli_x_->setValue(StampPara::DEFAULT_X);

    ui.hori_sli_y_->setMinimum(0);
    ui.hori_sli_y_->setMaximum(StampPara::MAX_Y);
    ui.hori_sli_y_->setValue(StampPara::DEFAULT_Y);

    ui.hori_sli_angle_->setMinimum(0);
    ui.hori_sli_angle_->setMaximum(StampPara::MAX_ANGLE);
    ui.hori_sli_angle_->setValue(StampPara::DEFAULT_ANGLE);

    QString str = ui.le_abc_stamper_idx_->text();
    if (str.isEmpty())
        ui.le_abc_stamper_idx_->setText(QString::number(1));
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

void QtDemo::HandleSliderX(int val)
{
    int pos = para_.x = ui.hori_sli_x_->value();
    QString str = QString("%1").arg(pos);
    ui.le_stamp_x_->setText(str);
}

void QtDemo::HandleSliderY(int val)
{
    int pos = para_.y = ui.hori_sli_y_->value();
    QString str = QString("%1").arg(pos);
    ui.le_stamp_y_->setText(str);
}

void QtDemo::HandleSliderAngle(int val)
{
    if (0 == para_.angle)
        return ui.le_stamp_angle_->setText("0");

    int pos = para_.angle = ui.hori_sli_angle_->value();
    QString str = QString("%1").arg(pos);
    ui.le_stamp_angle_->setText(str);
}

void QtDemo::HandleSelSeal()
{
    if (serial_ == 0) {
        std::srand(std::time(0));
        serial_ = std::rand();
    }

    int ret = GetStamperID(para_.stamp_idx - 1, para_.rfid);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取印章ID失败"));

    ret = FSelectStamper(
        serial_, 
        para_.rfid,
        para_.ink);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("选章失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("选章成功"), STATUS_TEXT);
}

void QtDemo::HandleOridinary()
{
    if (!IsOpened())
        return;

    int ret = GetStamperID(para_.stamp_idx - 1, para_.rfid);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("获取印章ID失败"));

    STAMPERPARAM pa;
    memcpy(&pa.seal, &para_.rfid, 4);

    std::srand(std::time(0));
    unsigned int serial = std::rand();
    pa.serial_number = serial;
    pa.isPadInk = para_.ink;
    pa.x_point = para_.x;
    pa.y_point = para_.y;
    para_.wait = atoi(ui.le_wait_time_->text().toStdString().c_str());
    pa.w_time = para_.wait;
    pa.type = 0;

    ret = FStartStamperstrc(&pa);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("普通盖章失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("普通盖章成功"), STATUS_TEXT);
}

void QtDemo::HandleConfirmStamp()
{
    if (!IsOpened())
        return;

    if (serial_ == 0) {
        return Info(QString::fromLocal8Bit("请先选章"));
    }

    para_.wait = atoi(ui.le_wait_time_->text().toStdString().c_str());
    int ret = ::Confirm(
        serial_,
        para_.rfid,
        para_.ink,
        para_.x,
        para_.y,
        0,
        para_.wait);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("确认盖章失败"));

    serial_ = 0;
    ui.statusBar->showMessage(QString::fromLocal8Bit("确认盖章成功"), STATUS_TEXT);
}

void QtDemo::HandleCheckMark()
{
    if (!IsOpened())
        return;

    if (serial_ == 0) {
        return Info(QString::fromLocal8Bit("请先选章"));
    }

    para_.wait = atoi(ui.le_wait_time_->text().toStdString().c_str());
    std::srand(std::time(0));
    int ret = ::Confirm(
        std::rand(),
        para_.stamp_idx,
        para_.ink,
        para_.x,
        para_.y,
        para_.angle,
        para_.wait,
        1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("确认盖骑缝章失败"));

    serial_ = 0;
    ui.statusBar->showMessage(QString::fromLocal8Bit("确认盖骑缝章成功"), STATUS_TEXT);
}

void QtDemo::HandldeCancelStamp()
{
    if (!IsOpened())
        return;

    int ret = ::FCancleStamper();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("取消盖章失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("取消盖章成功"), STATUS_TEXT);
}

void QtDemo::HandleReadStamp()
{
    unsigned char info[48] = { 0 };
    int ret = ReadStamp(info, 47);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("读盖章信息失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("读盖章信息成功"), STATUS_TEXT);
}

void QtDemo::HandleReadABC()
{
    std::string stamper = ui.le_abc_stamper_idx_->text().toStdString();
    char id[13] = { 0 };
    unsigned long begin = GetTickCount();
    int ret = GetABCStamper(atoi(stamper.c_str()), id, 12);
    ShowTimeElapsed(begin);

    if (0 != ret) {
        return Info(QString::fromLocal8Bit("读农行电子标签失败, ") + QString::number(ret));
    }

    ui.le_abc_stamper_->clear();
    ui.le_abc_stamper_->setText(QString::fromStdString(id));
    ui.le_abc_stamper_->adjustSize();
    ui.statusBar->showMessage(QString::fromLocal8Bit("读农行电子标签成功"), STATUS_TEXT);
}

void QtDemo::HandleReadABCIndex()
{
    char index = 0;
    QString str = ui.le_abc_stamper_->text();
    int ret = GetABCStamperIndex(str.toStdString().c_str(), &index);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("读农行印章仓位号失败"));

    ui.le_abc_stamper_idx_->clear();
    ui.le_abc_stamper_idx_->setText(QString::number(index));
    Info(QString::fromLocal8Bit("读农行印章仓位号成功"));
}

///////////////////////// RFID 操作 //////////////////////////////

void QtDemo::RfidStamper1()
{
    ui.rb_stamper1_->setChecked(true);
}

void QtDemo::RfidStamper2()
{
    ui.rb_stamper2_->setChecked(true);
}

void QtDemo::RfidStamper3()
{
    ui.rb_stamper3_->setChecked(true);
}

void QtDemo::RfidStamper4()
{
    ui.rb_stamper4_->setChecked(true);
}

void QtDemo::RfidStamper5()
{
    ui.rb_stamper5_->setChecked(true);
}

void QtDemo::RfidStamper6()
{
    ui.rb_stamper6_->setChecked(true);
}

//rfid页初始化
void QtDemo::InitRfidPage()
{
    ui.combo_key_type_->addItem(QString::fromLocal8Bit("A密码"));
    ui.combo_key_type_->addItem(QString::fromLocal8Bit("B密码"));
    ui.combo_key_type_->setCurrentIndex(0);
    ui.line_key_->setText("FF FF FF FF FF FF");
    ui.le_block_add_->setText("10");
}

int QtDemo::RadioButtonSelected(QRadioButton** btns, int size)
{
    for (int i = 0; i < size; ++i) {
        if (btns[i]->isChecked())
            return i;
    }

    return -1;
}

void QtDemo::HandleRfidTutorial()
{
    Info(QString::fromLocal8Bit("按以下步骤操作:\n"
        "1. 卡选择; \n"
        "2. 卡请求; \n"
        "3. 设置地址;\n"
        "4. 密码校验;\n"
        "5. 卡读写"));
}

void QtDemo::HandleSelectStamper()
{
    QRadioButton* btns[] = 
    {
        ui.rb_stamper1_, 
        ui.rb_stamper2_, 
        ui.rb_stamper3_, 
        ui.rb_stamper4_,
        ui.rb_stamper5_, 
        ui.rb_stamper6_
    };
    int stamper = RadioButtonSelected(btns, 6);
    if (stamper < 0)
        return Info(QString::fromLocal8Bit("未选择有效章"));

    rfid_.stamper = (unsigned char)stamper;
    int ret = SelectStamper(rfid_.stamper); //卡选择
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("卡选择失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("卡选择成功"), STATUS_TEXT);
}

void QtDemo::HandleGetRFID()
{
    int ret = GetStamperID(rfid_.stamper, rfid_.rfid); //卡请求
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("卡请求失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("卡请求成功"), STATUS_TEXT);
}

void QtDemo::HandleAllaStamperStatus()
{
    if (!IsOpened())
        return;

    QRadioButton* btns[] =
    {
        ui.rb_stamper1_,
        ui.rb_stamper2_,
        ui.rb_stamper3_,
        ui.rb_stamper4_,
        ui.rb_stamper5_,
        ui.rb_stamper6_,
        ui.rb_side_door_
    };

    for (int i = 0; i < 7; ++i) {
        unsigned int rfid = 0;
        int ret = GetStamperID(i, rfid);
        if (STF_SUCCESS != ret)
            continue;

        char str[1024] = { 0 };
        if (rfid == 0) {
            sprintf(str, "<%d>号章无<RFID>", i + 1);
        } else {
            rfid_.stamper_rfid.insert(std::make_pair(i, rfid));
            if (i == 6)
                sprintf(str, "侧门锁章: <%d>号有<RFID>", i + 1);
            else
                sprintf(str, "<%d>号章有<RFID>", i + 1);
        }
        btns[i]->setText(QString::fromLocal8Bit(str));
        btns[i]->adjustSize();
    }
}

void QtDemo::HandleRequestAllCard()
{
    if (!IsOpened())
        return;

    QRadioButton* btns[] =
    {
        ui.rb_stamper1_,
        ui.rb_stamper2_,
        ui.rb_stamper3_,
        ui.rb_stamper4_,
        ui.rb_stamper5_,
        ui.rb_stamper6_,
        ui.rb_side_door_
    };

    for (int i = 0; i < 7; ++i) {
        unsigned int rfid = rfid_.stamper_rfid.find(i)->second;
        char str[1024] = { 0 };
        if (i == 6)
            sprintf(str, "侧门锁章: <%d>号有<RFID>:0x%x", i + 1, rfid);
        else
            sprintf(str, "<%d>号章有<RFID>:0x%x", i + 1, rfid);
        btns[i]->setText(QString::fromLocal8Bit(str));
    }
}

//设置卡绝对地址
void QtDemo::HandleBtnSetBlockAddr()
{
    std::string str = ui.le_block_add_->text().toStdString();
    rfid_.block = atoi(str.c_str());
    int ret = OperateBlock(rfid_.block); //卡设置绝对地址
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("卡设置绝对地址失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("卡设置绝对地址成功"), STATUS_TEXT);
}

void QtDemo::HexStr2Decimal(std::string hex_str, unsigned char* decimal_str, unsigned int len)
{
#define BLANK ' '
    std::string str = hex_str;

    int i = 0;
    int idx = 0;
    while (i < len) {
        size_t blank_pos = str.find_first_of(BLANK);
        if (blank_pos == std::string::npos) {
            char* str_copy = new char[str.length() + 1];
            memcpy(str_copy, str.c_str(), str.length());
            int val = 0;
            sscanf(str_copy, "%x", &val);
            decimal_str[idx++] = val;
            delete[] str_copy;

            break;
        } else {
            std::string sub = str.substr(0, blank_pos);
            char* sub_copy = new char[sub.length() + 1];
            memcpy(sub_copy, sub.c_str(), sub.length());
            int val = 0;
            sscanf(sub_copy, "%x", &val);
            decimal_str[idx++] = val;
            delete[] sub_copy;

            str = str.substr(blank_pos + 1, std::string::npos);
        }

        ++i;
    }
}

//卡密码校验
void QtDemo::HandleVerifyKey()
{
    //获取密钥类型和密钥
    std::string str_key = ui.line_key_->text().toStdString();
    unsigned char key[6] = { 0 };
    HexStr2Decimal(str_key, key, 6);
    memcpy(rfid_.key, key, sizeof(key));

    int ret = VerifyKey(rfid_.key_type, rfid_.key, 6);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("卡密码校验失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("卡密码校验成功"), STATUS_TEXT);
}

//RFID出厂密码
void QtDemo::HandleRFIDFactoryCode()
{
    ui.line_key_->setText("FF FF FF FF FF FF");
}

//切换密钥类型
void QtDemo::HandleKeyType(int index)
{
    rfid_.key_type = ui.combo_key_type_->currentIndex();
}

//写块
void QtDemo::HandleWriteBlock()
{
    std::string str_data = ui.le_write_block_data_->text().toStdString();
    char data[6] = { 0 };
    HexStr2Decimal(str_data, (unsigned char*)data, 6);

    int ret = WriteBlock(rfid_.block, data, 3);
    if (STF_SUCCESS != ret)
        Info(QString::fromLocal8Bit("块写入失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("块写入成功"), STATUS_TEXT);
}

//读块
void QtDemo::HandleReadBlock()
{
    char data[17] = { 0 };
    int ret = ReadBlock(rfid_.block, data, 4);
    if (ret < 0)
        return Info(QString::fromLocal8Bit("读块失败"));

    std::string str;
    for (int i = 0; i < 4; ++i) {
        base::Number2HexStr<unsigned char> da(data[i]);
        str.append(da.UpperHexStr()).append(" ");
    }

    ui.le_read_block_data_->setText(QString::fromLocal8Bit(str.c_str()));
    ui.statusBar->showMessage(QString::fromLocal8Bit("读块成功"), STATUS_TEXT);
}

//////////////////////////// 其他接口 ///////////////////////////////

void QtDemo::InitOther()
{
    ui.le_sidedoor_close_->setText("30");
    ui.le_sidedoor_timeout_->setText("60");
}

void QtDemo::HandleLock()
{
    int ret = ::Lock();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("锁定印控仪失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功锁定印控仪"), STATUS_TEXT);
}

void QtDemo::HandleUnlock()
{
    int ret = ::Unlock();
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("解锁印控仪失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功解锁印控仪"), STATUS_TEXT);
}

void QtDemo::HandleIsLock()
{
    bool ret = IsLocked();
    if (!ret)
        return Info(QString::fromLocal8Bit("未锁定状态"));

    Info(QString::fromLocal8Bit("锁定状态"));
}

void QtDemo::HandleWriteID()
{
    std::string id = ui.le_write_id_->text().toStdString();
    if (id.empty())
        return Info(QString::fromLocal8Bit("输入编号为空"));

    int ret = ::WriteStamperIdentifier((unsigned char*)id.c_str(), id.length());
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("写编号失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("写编号成功"), STATUS_TEXT);
}

void QtDemo::HandleReadID()
{
    unsigned char id[512] = { 0 };
    int ret = ::ReadStamperIdentifier(id, 511);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("读编号失败"));

    ui.le_read_id_->setText(QString::fromStdString((char*)id));
    ui.statusBar->showMessage(QString::fromLocal8Bit("读编号成功"), STATUS_TEXT);
}

void QtDemo::HandleWriteBackupSN()
{
    std::string sn = ui.le_write_backup_sn_->text().toStdString();
    if (sn.empty())
        return Info(QString::fromLocal8Bit("序列号为空"));

    if (sn.length() > 48)
        return Info(QString::fromLocal8Bit("序列号超过最大长度"));

    int ret = WriteBackupSN((unsigned char*)sn.c_str(), sn.length());
    if (ret != STF_SUCCESS)
        return Info(QString::fromLocal8Bit("写备板序列号失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("写备板序列号成功"), STATUS_TEXT);
}

void QtDemo::HandleReadBackupSN()
{
    unsigned char sn[49] = { 0 };
    int ret = ReadBackupSN(sn, 48);
    if (ret != STF_SUCCESS)
        return Info(QString::fromLocal8Bit("读备板序列号失败"));

    ui.le_read_backup_sn_->setText(QString::fromStdString((char*)sn));
    ui.statusBar->showMessage(QString::fromLocal8Bit("读备板序列号成功"), STATUS_TEXT);
}

void QtDemo::HandleSetSideDoor()
{
    std::string time1 = ui.le_sidedoor_close_->text().toStdString();
    std::string time2 = ui.le_sidedoor_timeout_->text().toStdString();

    int ret = SetSideDoor(atoi(time1.c_str()), atoi(time2.c_str()));
    if (ret != STF_SUCCESS)
        return Info(QString::fromLocal8Bit("设置侧门开门提示失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置侧门开门提示成功"), STATUS_TEXT);
}

void QtDemo::HandleWriteMAC()
{
    std::string mac1 = ui.le_write_mac1_->text().toStdString();
    std::string mac2 = ui.le_write_mac2_->text().toStdString();

    int ret = WriteMAC(
        mac1.empty()? NULL: (unsigned char*)mac1.c_str(),
        mac2.empty()? NULL: (unsigned char*)mac2.c_str(),
        mac1.length(),
        mac2.length());
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("写MAC地址失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("写MAC地址成功"), STATUS_TEXT);
}

void QtDemo::ShowTimeElapsed(unsigned long begin)
{
    unsigned long end = GetTickCount();
    unsigned long interval = end - begin;
    BOOST_LOG_TRIVIAL(info) << "(begin, end) = (" << begin << ", " << end << ")";
    Info(QString::fromLocal8Bit("耗时(毫秒): ") + QString::number(interval));
}

void QtDemo::HandleReadMAC()
{
    unsigned char mac1[18] = { 0 };
    unsigned char mac2[18] = { 0 };
    unsigned long begin = GetTickCount();
    int ret = ReadMAC(
        mac1,
        mac2,
        18);
    ShowTimeElapsed(begin);

    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("读MAC地址失败"));

    ui.le_read_mac1_->setText(QString::fromStdString((char*)mac1));
    ui.le_read_mac2_->setText(QString::fromStdString((char*)mac2));
    ui.statusBar->showMessage(QString::fromLocal8Bit("读MAC地址成功"), STATUS_TEXT);
}

void QtDemo::HandleReadVoltage()
{
    unsigned char voltage = 0;
    int ret = ReadAlarmVoltage(&voltage);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("读报警器电压失败"));

    ui.le_voltage_->setText(QString::fromLocal8Bit("电压值(伏): ") + QString::number(voltage * 0.1));
    ui.le_voltage_->adjustSize();
    ui.statusBar->showMessage(QString::fromLocal8Bit("读报警器电压成功"), STATUS_TEXT);
}

void QtDemo::HandldeWriteCalPoints()
{
    unsigned short pts[] = 
    {
        12, 34,
        54, 104,
        209, 45,
        232, 13,
        101, 110
    };

    int ret = WriteCalibrationPoint(pts, 10);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("写校准点失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("写校准点成功"), STATUS_TEXT);
}

void QtDemo::HandleReadPCalPoints()
{
    unsigned short pts[10] = { 0 };
    int ret = CalibrationPoint(pts, 10);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("读校准点失败"));

    QString str;
    str = QString::fromLocal8Bit("校准点:\n") + 
        QString::fromLocal8Bit("(") + QString::number(pts[0]) + QString::fromLocal8Bit(",") + QString::number(pts[1]) + QString::fromLocal8Bit(")\n") +
        QString::fromLocal8Bit("(") + QString::number(pts[2]) + QString::fromLocal8Bit(",") + QString::number(pts[3]) + QString::fromLocal8Bit(")\n") +
        QString::fromLocal8Bit("(") + QString::number(pts[4]) + QString::fromLocal8Bit(",") + QString::number(pts[5]) + QString::fromLocal8Bit(")\n") +
        QString::fromLocal8Bit("(") + QString::number(pts[6]) + QString::fromLocal8Bit(",") + QString::number(pts[7]) + QString::fromLocal8Bit(")\n") +
        QString::fromLocal8Bit("(") + QString::number(pts[8]) + QString::fromLocal8Bit(",") + QString::number(pts[9]) + QString::fromLocal8Bit(")");
    
    Info(str);
    ui.statusBar->showMessage(QString::fromLocal8Bit("读校准点成功"), STATUS_TEXT);
}

void QtDemo::HandleReadRange()
{
    char physical[12] = { 0 };
    int ret = GetPhsicalRange(physical, 12);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("读盖章物理范围失败"));

    unsigned short x_width = 0;
    memcpy(&x_width, physical, sizeof(unsigned short));

    unsigned short y_width = 0;
    memcpy(&y_width, physical + sizeof(unsigned short), sizeof(unsigned short));

    unsigned short x_min = 0;
    memcpy(&x_min, physical + 2 * sizeof(unsigned short), sizeof(unsigned short));

    unsigned short x_max = 0;
    memcpy(&x_max, physical + 3 * sizeof(unsigned short), sizeof(unsigned short));

    unsigned short y_min = 0;
    memcpy(&y_min, physical + 4 * sizeof(unsigned short), sizeof(unsigned short));

    unsigned short y_max = 0;
    memcpy(&y_max, physical + 5 * sizeof(unsigned short), sizeof(unsigned short));

    ui.le_x_width_->setText(QString::number(x_width));
    ui.le_y_width_->setText(QString::number(y_width));
    ui.le_x_min_->setText(QString::number(x_min));
    ui.le_x_max_->setText(QString::number(x_max));
    ui.le_y_min_->setText(QString::number(y_min));
    ui.le_y_max_->setText(QString::number(y_max));

    ui.statusBar->showMessage(QString::fromLocal8Bit("读盖章物理范围成功"), STATUS_TEXT);
}

void QtDemo::HandleReadAlarm()
{
    char door = -1;
    char vibration = -1;
    int ret = ReadAlarmStatus(&door, &vibration);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("读报警器控制状态失败, err:") + QString::number(ret));

    ui.label_door_alarm_->setText(
        0 == door ? QString::fromLocal8Bit("关"): QString::fromLocal8Bit("开"));
    ui.label_vibration_alarm_->setText(
        0 == vibration ? QString::fromLocal8Bit("关") : QString::fromLocal8Bit("开"));
    ui.label_door_alarm_->adjustSize();
    ui.label_door_alarm_->adjustSize();
    ui.statusBar->showMessage(QString::fromLocal8Bit("读报警器控制状态成功"), STATUS_TEXT);
}

void QtDemo::HandleHardwareVer()
{
    char version[255] = { 0 };
    int ret = GetHardwareVer(version, 254);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("读硬件版本号失败, err:") + QString::number(ret));

    Info(QString::fromStdString(version));
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
        case CMD_UPDATE_PROGRESS:
            ui.pb_display_progress_->setValue(msg->val_ * 1000 / msg->obj_);
            break;
        default:
            break;
        }
    }

    delete msg;
}

void QtDemo::StartTimer(uint16_t milliseconds)
{
    timer_ = new QTimer(this);
    connect(timer_, SIGNAL(timeout()), this, SLOT(TimerDone()));
    timer_->start(milliseconds); // 1秒单触发定时器
}

void QtDemo::PushMessage(Message* msg)
{
    msg_queue_.Push(msg);
}

void QtDemo::OpenDevPost(const Message* msg)
{
    open_called = msg->err_ == 0;
    ui.label_show_dev_->setText(cmd_des[msg->err_ == 0 ? OC_OPEN_DEV_SUC: OC_OPEN_DEV_FAIL]);
    ui.label_show_dev_->adjustSize();

//     int ret = SetAlarm(1, 0);
//     if (0 == ret)
//         ui.statusBar->showMessage(QString::fromLocal8Bit("成功关闭振动报警"), STATUS_TEXT);
//     else
//         ui.statusBar->showMessage(QString::fromLocal8Bit("关闭振动报警失败"), STATUS_TEXT);

//     int loop = 1;
//     while (loop--) {
//         ret = Reset();
//         if (0 != ret)
//             Info(QString::fromLocal8Bit("复位失败"));
//         else
//             ui.statusBar->showMessage(QString::fromLocal8Bit("复位成功"), STATUS_TEXT);
//     }
// 
//     loop = 2;
//     while (loop--) {
//         ret = SetStampMap();
//         if (0 != ret)
//             Info(QString::fromLocal8Bit("设置章映射失败"));
//         else
//             ui.statusBar->showMessage(QString::fromLocal8Bit("设置章映射成功"), STATUS_TEXT);
//     }
}

void QtDemo::CloseDevPost(const Message* msg)
{
    open_called = msg->err_ != 0;
    ui.label_show_dev_->setText(cmd_des[msg->err_ == 0? OC_CLOSE_DEV_SUC: OC_CLOSE_DEV_FAIL]);
    ui.label_show_dev_->adjustSize();
}
