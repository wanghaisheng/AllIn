#include "qtdemo.h"
#include <QtWidgets/QMessageBox>
#include <QFileDialog>
#include <QTextCodec>
#include <QDebug>
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
        break;//设备断开连接，处理在回调函数
    case 1:
    {//设备连接	
        //更新界面
        setWindowTitle("设备打开成功");
    }
    break;
    default:
        break;
    }
}

QtDemo::QtDemo(QWidget *parent)
    : QMainWindow(parent), echo_thread_(NULL), open_called(true), test_mode_(false),
    img_(NULL), serial_(0), capture_suc(false)
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

    // to-do , 印章校准

    connect(ui.le_err_code_, SIGNAL(textChanged(const QString &)), this, 
        SLOT(HandleErrCodeChange(const QString &)));


    //拍照/识别
    ui.pb_ori_img_->setStyleSheet("background-color: transparent;");
    ui.pb_cut_img_->setStyleSheet("background-color: transparent;");
    connect(ui.pb_ori_img_, &QPushButton::clicked, this, &QtDemo::HandleOriImgClick);
    connect(ui.pb_cut_img_, &QPushButton::clicked, this, &QtDemo::HandleCutImgClick);

    connect(ui.pb_capture_, &QPushButton::clicked, this, &QtDemo::HandleCapture);
    connect(ui.pb_select_picture_, &QPushButton::clicked, this, &QtDemo::HandleSelectPic);

    connect(ui.pb_illustrate_, &QPushButton::clicked, this, &QtDemo::HandleIllusrate);
    connect(ui.pb_check_stamp_, &QPushButton::clicked, this, &QtDemo::HandleCheckStamp);

    connect(ui.combo_img_sel_, SIGNAL(activated(int)), this, SLOT(HandleSelectImg(int)));
    connect(ui.pb_recog_code_, &QPushButton::clicked, this, &QtDemo::HandleRecogCode);
    connect(ui.pb_recog_ele_, &QPushButton::clicked, this, &QtDemo::HandleRecogEle);

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
    connect(ui.pb_auto_stamp_, &QPushButton::clicked, this, &QtDemo::HandleAutoStamp);

    connect(ui.pb_finish_stamp_, &QPushButton::clicked, this, &QtDemo::HandleFinishStamp);
    connect(ui.pb_release_machine_, &QPushButton::clicked, this, &QtDemo::HandleReleaseMachine);

    // 其他接口
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
    
    // 默认显示第一个tab
    ui.tabWidget->setCurrentIndex(0);
    ui.tabWidget->removeTab(3);
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

    StartTimer(1000);

    InitDevControl(); //需要手动调用一次
    return true;
}

QtDemo::~QtDemo()
{
    char cmd[256] = { 0 };
    sprintf_s(cmd, "taskkill /f /in %s", "mc_exed.exe");
    system(cmd);
}

/////////////////////////// 设备控制 ////////////////////////////////

void QtDemo::InitDevControl()
{
    if (register_conn_cb_ != 0)
        BOOST_LOG_TRIVIAL(error) << "注册设备回掉函数失败";
    if (register_msg_cb_ != 0)
        BOOST_LOG_TRIVIAL(error) << "注册回掉函数(盖章结果)失败";

    ui.le_stamper_idx_->setText(QString::number(1)); //从1开始
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

    int ret = BindMAC(mac);
    return Info(QString::fromLocal8Bit("绑定MAC地址, er: ") + QString::number(ret));
}

void QtDemo::HandleUnbinding()
{
    std::string mac = ui.le_mac_->text().toStdString();
    if (mac.empty())
        return Info(QString::fromLocal8Bit("待解绑MAC地址为空"));

    int ret = UnbindMAC(mac);
    return Info(QString::fromLocal8Bit("解绑MAC地址, er: ") + QString::number(ret));
}

void QtDemo::HandleQueryMAC()
{
    std::string mac1;
    std::string mac2;
    int ret = QueryMAC(mac1, mac2);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("查询MAC地址失败, er: ") + QString::number(ret));

    ui.le_mac1_->clear();
    ui.le_mac2_->clear();

    ui.le_mac1_->setText(QString::fromStdString(mac1));
    ui.le_mac2_->setText(QString::fromStdString(mac2));
}

void QtDemo::HandleOpenSafeDoorAlarm()
{
    int ret = ControlAlarm(0, 1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开安全门报警失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功打开安全门报警"), STATUS_TEXT);
}

void QtDemo::HandleCloseSafeDoorAlarm()
{
    int ret = ControlAlarm(0, 0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭安全门报警失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功关闭安全门报警"), STATUS_TEXT);
}

void QtDemo::HandleOpenVibrationAlarm()
{
    int ret = ControlAlarm(1, 1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开振动报警失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功打开振动报警"), STATUS_TEXT);
}

void QtDemo::HandleCloseVibrationAlarm()
{
    int ret = ControlAlarm(1, 0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭振动报警失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("成功关闭振动报警"), STATUS_TEXT);
}

void QtDemo::HandleQuerySN()
{
    std::string sn;
    int ret = QueryMachine(sn);
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

    int ret = SetMachine(sn);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("设置设备编号失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("设置设备编号成功"), STATUS_TEXT);
}

void QtDemo::HandleOpenSafeDoor()
{
    int ctrl = 1;
    int ret = ControlSafe(ctrl);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("打开安全门失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("打开安全门成功"), STATUS_TEXT);
}

void QtDemo::HandleCloseSafeDoor()
{
    int ctrl = 0;
    int ret = ControlSafe(ctrl);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("关闭安全门失败"));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关闭安全门成功"), STATUS_TEXT);
}

void QtDemo::HandleReadSafe()
{
    int safe_status;
    int ret2 = QuerySafe(safe_status);
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
    int ret1 = QueryPaper(paper_status);
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
    int ret = ControlBeep(1);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("打开蜂鸣器失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("打开蜂鸣器成功"), STATUS_TEXT);
}

void QtDemo::HandleBeepOff()
{
    int ret = ControlBeep(0);
    if (STF_SUCCESS != ret)
        return Info(QString::fromLocal8Bit("关闭蜂鸣器失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("关闭蜂鸣器成功"), STATUS_TEXT);
}

void QtDemo::HandleQuerySlots()
{
    int num;
    int ret = QuerySlot(num);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("查询卡槽数量失败, err: ") + QString::number(num));

    Info(QString::fromLocal8Bit("卡槽数量: ") + QString::number(num));
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

void QtDemo::HandleErrCodeChange(const QString & txt)
{
    std::string err_str = txt.toStdString();
    for (int i = 0; i < err_str.length(); ++i) {
        if (err_str.at(i) < 0x30 || err_str.at(i) > 0x39)
            return Info(QString::fromLocal8Bit("错误码应为正整数, 请重新输入"));
    }

    int err = atoi(err_str.c_str());
    std::string msg, resolver;
    int ret = GetError(err, msg, resolver);
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
    int ret = Snapshot(
        200,
        200,
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

void QtDemo::HandleRecogCode()
{
    // 清空之前结果
    ui.la_code_result_->clear();

    std::string template_id;
    std::string trace_num;
    int ret;
    if (0 == img_type_) {
        ret = RecognizeImage(Config::GetInst()->ori_path_, template_id, trace_num);
    }
    else if (1 == img_type_) {
        ret = RecognizeImage(Config::GetInst()->cut_path_, template_id, trace_num);
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
    int ret = IdentifyElement(
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

void QtDemo::HandlePreStamp()
{

}

void QtDemo::InitSnapshot()
{
    ui.combo_img_sel_->clear();
    ui.combo_img_sel_->addItem(QString::fromLocal8Bit("原图"));
    ui.combo_img_sel_->addItem(QString::fromLocal8Bit("切图"));
    ui.combo_img_sel_->setFixedSize(111, 22);

    ui.le_x_in_dev_->setText(QString::number(3));
    ui.le_y_in_dev_->setText(QString::number(60));

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
    std::string voucher;
    int ret = OrdinaryStamp(
        ui.le_task_id_->text().toStdString(),
        voucher,
        para_.stamp_idx,
        atoi(ui.le_x_in_img_->text().toStdString().c_str()),    // 盖章位置x坐标, 原始图片中的像素
        atoi(ui.le_y_in_img_->text().toStdString().c_str()),    // 盖章位置y坐标, 原始图片中的像素
        0);                                                     // 印章旋转角度, 大于等于0且小于360度
    if (0 != ret)
        return Info(QString::fromLocal8Bit("普通用印失败"));

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
    int ret = PrepareStamp(
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

    int ret = FinishStamp(task_id);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("结束用印失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("结束用印成功"), STATUS_TEXT);
}

void QtDemo::HandleReleaseMachine()
{
    std::string machine = ui.le_show_sn_->text().toStdString();
    if (machine.empty())
        return Info(QString::fromLocal8Bit("请先通过\"设备控制\"页获取设备编号"));

    int ret = ReleaseStamp(machine);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("释放印控机失败, er: ") + QString::number(ret));

    ui.statusBar->showMessage(QString::fromLocal8Bit("释放印控机成功"), STATUS_TEXT);
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
    std::string sn;
    int ret = QueryMachine(sn);
    if (0 != ret)
        return Info(QString::fromLocal8Bit("读编号失败"));

    ui.le_read_id_->setText(QString::fromStdString(sn));
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
}

void QtDemo::CloseDevPost(const Message* msg)
{
    open_called = msg->err_ != 0;
}
