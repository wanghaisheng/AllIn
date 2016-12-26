#include "qtdemo.h"
#include <QtWidgets/QMessageBox>
#include <QFileDialog>
#include <QTextCodec>
#include <fstream>
#include "USBControlF60.h"
#include "SealLog.h"
#include "RZCamera.h"
#include "common.h"
#include "event.h"

extern QString font_family;
extern int font_size;
extern std::string logo;
extern QColor update_hint;
extern QColor update_succ;

boost::mutex QtDemo::mtx;
int QtDemo::connected = -1;

bool GetMoudulePath(std::string& path)
{
    TCHAR file_path[_MAX_PATH] = { 0 };
    if (GetModuleFileName(NULL, file_path, _MAX_FNAME) == 0)
        return false;

    int iLen = WideCharToMultiByte(CP_ACP, 0, file_path, -1, NULL, 0, NULL, NULL);
    char* chRtn = new char[iLen*sizeof(char)];
    WideCharToMultiByte(CP_ACP, 0, file_path, -1, chRtn, iLen, NULL, NULL);
    std::string file_path_str(chRtn);
    delete[] chRtn;

    size_t last_slash = file_path_str.find_last_of("\\");
    if (last_slash == std::string::npos)
        return false;

    path = file_path_str.substr(0, last_slash + 1);
    return true;
}

int QtDemo::ConnectCallBack(const char* path, unsigned int msg)
{
    switch (msg) {
    case 0: {
        ::FCloseDev();
        WriteSealLog(4, "QtDemo::ConnectCallBack->设备已断开");
    }
        break;
    case 1: {
        int ret = ::FOpenDev(path);
        if (0 == ret)
            WriteSealLog(4, "QtDemo::ConnectCallBack->设备重连成功");
        else
            WriteSealLog(4, "QtDemo::ConnectCallBack->设备重连失败");
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
        //QString msg = "电子锁" + QString::fromLocal8Bit("关闭");
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
        setWindowTitle("设备断开");
        ::FCloseDev();
        break;//设备断开连接，处理在回调函数
    case 1:
    {//设备连接	
        //更新界面
        setWindowTitle("设备已连接");
        ::FOpenDev(path);
        setWindowTitle("设备打开成功");
    }
    break;
    default:
        break;
    }
}

QtDemo::QtDemo(QWidget *parent)
    : QMainWindow(parent), echo_thread_(NULL), open_called(false), count_(0)
{
    ui.setupUi(this);
    setWindowTitle(DIALOG_HEADER);
    ui.logo_img_->hide();

    this->setAutoFillBackground(true);
    // 设置窗口背景色
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(255, 251, 240));
    //palette.setBrush(QPalette::Background, QBrush(QPixmap(":/background.png")));
    this->setPalette(palette);

    register_conn_cb_ = F_RegisterDevCallBack(QtDemo::ConnectCallBack);
    register_msg_cb_ = FRegisterDevCallBack(QtDemo::DevMsgCallBack);

    if (ParseXML()) {
        // 解析配置文件成功, 修改相关配置预设值
        font_family = QString::fromStdString(xml_pt_.get<std::string>("config.font.family"));
        font_size = atoi(xml_pt_.get<std::string>("config.font.size").c_str());

        logo = xml_pt_.get<std::string>("config.logo");

        update_hint = QColor(
            atoi(xml_pt_.get<std::string>("config.hint.R").c_str()),
            atoi(xml_pt_.get<std::string>("config.hint.G").c_str()),
            atoi(xml_pt_.get<std::string>("config.hint.B").c_str()));

        update_succ = QColor(
            atoi(xml_pt_.get<std::string>("config.succ.R").c_str()),
            atoi(xml_pt_.get<std::string>("config.succ.G").c_str()),
            atoi(xml_pt_.get<std::string>("config.succ.B").c_str()));
    }
    else {
        WriteSealLog(3, "解析配置文件失败");
    }

    // signals & slots
    connect(this, SIGNAL(ConnectStatus(const char*, unsigned int)), this, 
        SLOT(HandleConnect(const char*, unsigned int)));

    //主界面
    connect(ui.btn_open_dev, &QPushButton::clicked, this, &QtDemo::HandleBtnOpenDev);
    connect(ui.btn_close_dev, &QPushButton::clicked, this, &QtDemo::HandleBtnCloseDev);

    //设备控制页
    connect(ui.pb_redetect_, &QPushButton::clicked, this, &QtDemo::HandleRedetect);
    connect(ui.pb_error_des_, &QPushButton::clicked, this, &QtDemo::HandleViewError);

    // 摄像头预览
    connect(ui.paper_cam_preview_,  &QPushButton::clicked, this, &QtDemo::HandlePreviewPaper);
    connect(ui.env_cam_preview_,    &QPushButton::clicked, this, &QtDemo::HandlePreviewEnv);
    connect(ui.safe_cam_preview_,   &QPushButton::clicked, this, &QtDemo::HandlePreviewSafe);

    //connect(ui.logo_img_, &QPushButton::clicked, this, &QtDemo::HandleLogoClicked);

    status_label_ = new QLabel(this);
    ui.statusBar->addWidget(status_label_);
    ui.statusBar->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
}

bool QtDemo::Init()
{
    InitDevControl(); // 需要手动调用一次
    return true;
}

void QtDemo::AutoOpen()
{
    HandleBtnOpenDev();
}

QtDemo::~QtDemo()
{
}

bool QtDemo::ParseXML()
{
    std::string path;
    if (!GetMoudulePath(path))
        return false;

    std::string xml_path = path + "config.xml";
    try {
        boost::property_tree::xml_parser::read_xml(xml_path, xml_pt_);
        return true;
    }
    catch (...) {
        boost::exception_ptr e = boost::current_exception();
        std::cout << boost::current_exception_diagnostic_information();
        char buf[2048] = { 0 };
        sprintf_s(buf, "QtDemo::ParseXML->异常: %s", boost::current_exception_diagnostic_information().c_str());
        WriteSealLog(3, buf);
        return false;
    }
}

/////////////////////////// 主界面 ////////////////////////////////

// 打开设备
void QtDemo::HandleBtnOpenDev()
{
    if (open_called) {
        WriteSealLog(4, "QtDemo::HandleBtnOpenDev->设备已打开");
        return;
    }

    int ret = FOpenDev(NULL);
    if (0 == ret) {
        ui.paper_cam_preview_->setDisabled(false);
        ui.env_cam_preview_->setDisabled(false);
        ui.safe_cam_preview_->setDisabled(false);

        WriteSealLog(4, "QtDemo::HandleBtnOpenDev->设备成功打开, 开启定时器");
        open_called = true;
        StartTimer(1000);
    } else {
        ui.paper_cam_preview_->setDisabled(true);
        ui.env_cam_preview_->setDisabled(true);
        ui.safe_cam_preview_->setDisabled(true);

        WriteSealLog(4, "QtDemo::HandleBtnOpenDev->设备打开失败");
        Info(QString::fromLocal8Bit("设备打开失败(请检查设备连接, 是否开机), 然后点击\"重新检测\"重试."));
    }
}

// 关闭设备
void QtDemo::HandleBtnCloseDev()
{
    int ret = FCloseDev();
    if (0 == ret) {
        WriteSealLog(4, "QtDemo::HandleBtnCloseDev->设备成功关闭, 停止定时器");
        open_called = false;
        timer_->stop();
    } else {
        WriteSealLog(4, "QtDemo::HandleBtnCloseDev->设备关闭失败");
    }
}

/////////////////////////// 设备控制 ////////////////////////////////

void QtDemo::ClearBackground()
{
    ui.dev_code_->setPalette(default_palette_);
    ui.dev_status_->setPalette(default_palette_);

    ui.safe_door_status_->setPalette(default_palette_);
    ui.paper_door_status_->setPalette(default_palette_);
    ui.top_door_status_->setPalette(default_palette_);

    ui.door_alarm_status_->setPalette(default_palette_);
    ui.vibration_alarm_status_->setPalette(default_palette_);

    ui.slot1_->setPalette(default_palette_);
    ui.slot2_->setPalette(default_palette_);
    ui.slot3_->setPalette(default_palette_);
    ui.slot4_->setPalette(default_palette_);
    ui.slot5_->setPalette(default_palette_);
    ui.slot6_->setPalette(default_palette_);

    ui.err_code_->setPalette(default_palette_);

    ui.paper_cam_status_->setPalette(default_palette_);
    ui.env_cam_status_->setPalette(default_palette_);
    ui.safe_cam_status_->setPalette(default_palette_);
}

void QtDemo::InitDevControl()
{
    // 默认配色板
    default_palette_ = ui.dev_code_->palette();

#ifdef _DEBUG
    ui.pb_error_des_->setDisabled(false);
#else
    ui.pb_error_des_->setDisabled(true);
#endif

    // 更新"重新检测"按钮背景色
//     update_timer_ = new QTimer(this);
//     connect(update_timer_, SIGNAL(timeout()), this, SLOT(UpdateRedetectTimer()));
//     update_timer_->start(1000);

    ui.mainToolBar->hide();
    ui.btn_open_dev->hide();
    ui.btn_close_dev->hide();

    if (register_conn_cb_ != 0);
    if (register_msg_cb_ != 0);
}

void QtDemo::HandlePreviewPaper()
{
    int ret = FLightCtrl(2, 1);
    ret = FLightBrightness(2, 25);

    // 先关闭其他摄像头
    CloseCamera(ENVCAMERA);
    CloseCamera(SIDECAMERA);

    ret = OpenCamera(PAPERCAMERA);
    if (0 != ret)
        goto ERR;

    if (0 != StartPreview(
        PAPERCAMERA,
        ui.camera_preview_->width(),
        ui.camera_preview_->height(),
        (HWND)ui.camera_preview_->winId())) {
        goto ERR;
    }

ERR:
    WriteSealLog(4, "打开纸门摄像头预览失败");
}

void QtDemo::HandlePreviewEnv()
{
    CloseCamera(PAPERCAMERA);
    CloseCamera(SIDECAMERA);

    int ret = OpenCamera(ENVCAMERA);
    if (0 != StartPreview(
        ENVCAMERA,
        ui.camera_preview_->width(),
        ui.camera_preview_->height(),
        (HWND)ui.camera_preview_->winId()))
        goto ERR;

ERR:
    WriteSealLog(4, "打开环境摄像头预览失败");
}

void QtDemo::HandlePreviewSafe()
{
    CloseCamera(PAPERCAMERA);
    CloseCamera(ENVCAMERA);

    int ret = FLightCtrl(1, 1);
    ret = FLightBrightness(1, 23);

    ret = OpenCamera(SIDECAMERA);
    if (0 != StartPreview(
        SIDECAMERA,
        ui.camera_preview_->width(),
        ui.camera_preview_->height(),
        (HWND)ui.camera_preview_->winId()))
        goto ERR;

ERR:
    WriteSealLog(4, "打开侧门摄像头预览失败");
}

void QtDemo::HandleRedetect()
{
//     preview_dlg_ = new CameraPreview(this, QString::fromLocal8Bit("凭证摄像头预览"));
//     preview_dlg_->show();
//     return;

    // 设备未成功打开, 打开设备
    if (!open_called) {
        HandleBtnOpenDev();
    }

    if (!open_called) {
        WriteSealLog(4, "QtDemo::HandleRedetect->设备打开失败");
        return;
    }

    ClearBackground();

    char buf[512] = { 0 };
    sprintf_s(buf, "QtDemo::HandleRedetect->停止定时器前count_ = %d", count_);
    WriteSealLog(4, buf);

    // 设备成功打开
    timer_->stop();
    count_mtx_.lock();
    count_ = 0;
    count_mtx_.unlock();
    timer_->start();
    WriteSealLog(4, "QtDemo::HandleRedetect->定时器重新启动");

    // 状态栏提示
    ui.statusBar->showMessage(QString::fromLocal8Bit("检测中..."), INFINITE);
}

void QtDemo::HandleViewError()
{
    QString err;
    for (size_t i = 0; i < ec_vec_.size(); ++i) {
        err.append(QString::number(i + 1));
        err.append(QString::fromLocal8Bit(". 错误描述: ").append(err_des[ec_vec_.at(i)])
            .append(QString::fromLocal8Bit(", 解决方法: ")).append(err_resolver[ec_vec_.at(i)]));
        err.append("\n");
        err.append("\n");
    }

    Info(err);
}

//////////////////////////////////////////////////////////////////////////

void QtDemo::UpdateDeviceSN()
{
    QPalette palette;
    palette.setColor(QPalette::Background, update_hint);
    ui.dev_code_->setPalette(palette);

    unsigned char sn[49] = { 0 };
    int ret = ReadBackupSN(sn, 48);
    if (0 != ret) {
        WriteSealLog(4, "QtDemo::UpdateDeviceSN->获取设备序号失败");
        return;
    }

    std::string device_sn = "ABC_ST_SCM_F61_20160818_1_" + std::string((char*)sn);
    ui.dev_code_->setText(QString::fromLocal8Bit(device_sn.c_str()));
    ui.dev_code_->adjustSize();

    palette.setColor(QPalette::Background, update_succ);
    ui.dev_code_->setPalette(palette);
}

void QtDemo::UpdateDeviceStatus()
{
    QPalette palette;
    palette.setColor(QPalette::Background, update_hint);
    ui.dev_status_->setPalette(palette);

    unsigned char status[24] = { 0 };
    int ret = ::FGetDevStatus(status, sizeof(status));
    if (STF_SUCCESS != ret) {
        WriteSealLog(4, "QtDemo::UpdateDeviceStatus->查询设备状态失败");
        return;
    }

    // 设备状态机描述
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
    sprintf_s(des, "%s", status_des[status[1]]);
    ui.dev_status_->setText(QString::fromLocal8Bit(des));
    ui.dev_status_->adjustSize();

    palette.setColor(QPalette::Background, update_succ);
    ui.dev_status_->setPalette(palette);

    // 处理故障模式
    if (status[1] == 5) {
        ec_vec_.clear();

        // P1, 电机忙
        switch (status[0]) {
        case 0x01:
            ec_vec_.push_back(EC_X_BUSY);
            break;
        case 0x02:
            ec_vec_.push_back(EC_Y_BUSY);
            break;
        case 0x04:
            ec_vec_.push_back(EC_Z_BUSY);
            break;
        default:
            break;
        }

        // P6-P9四字节(32位), 判断红外故障
        int val;
        std::vector<int> err_idx;
        memcpy(&val, &status[5], sizeof(int));
        for (int i = 1; i < 32; ++i) {
            if ((val & 0x01) == 1) {
                printf("第%d位为1\n", i);
                err_idx.push_back(i);
            }

            val = val >> 1;
        }

        for (size_t i = 0; i < err_idx.size(); ++i) {
            ErrorCode ec = (ErrorCode)(err_idx.at(i) - 1);
            ec_vec_.push_back(ec);
        }

        // P16, 开机时检测章安全状态
        if (1 == status[14])
            ec_vec_.push_back(EC_STARTUP);

        // P25, 其他故障状态标志位
        // 第一位: 外部存储器故障
        // 第二位: 门超时未关蜂鸣器响起
        char other_er = status[24];
        if (other_er & 0x01)
            ec_vec_.push_back(EC_OUTSIZE_MEM);

        if (other_er & 0x02)
            ec_vec_.push_back(EC_DOOR_TIMEOUT);

        // 错误代码列表不为空, 显示查询故障原因按钮
        if (!ec_vec_.empty())
            ui.pb_error_des_->setDisabled(false);
        else
            ui.pb_error_des_->setDisabled(true);
    }
    else {
        ui.pb_error_des_->setDisabled(true);
    }
}

void QtDemo::UpdateDoors()
{
    QPalette palette;
    palette.setColor(QPalette::Background, update_hint);
    ui.safe_door_status_->setPalette(palette);
    ui.paper_door_status_->setPalette(palette);
    ui.top_door_status_->setPalette(palette);

    char door[4] = { 0 };
    int ret = ::FGetDoorsPresent(door, 4);
    if (STF_SUCCESS != ret) {
        WriteSealLog(4, "QtDemo::UpdateDoors->获取门状态失败");
        return;
    }

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
    ui.safe_door_status_->setText(QString::fromLocal8Bit(info[door[2]]));
    ui.safe_door_status_->adjustSize();

    ui.paper_door_status_->setText(QString::fromLocal8Bit(info[door[0]]));
    ui.paper_door_status_->adjustSize();

    ui.top_door_status_->setText(QString::fromLocal8Bit(info[door[3]]));
    ui.top_door_status_->adjustSize();

    palette.setColor(QPalette::Background, update_succ);
    ui.safe_door_status_->setPalette(palette);
    ui.paper_door_status_->setPalette(palette);
    ui.top_door_status_->setPalette(palette);
}

void QtDemo::UpdateAlarm()
{
    QPalette palette;
    palette.setColor(QPalette::Background, update_hint);
    ui.door_alarm_status_->setPalette(palette);
    ui.vibration_alarm_status_->setPalette(palette);

    char door = -1;
    char vibration = -1;
    int ret = ReadAlarmStatus(&door, &vibration);
    if (0 != ret) {
        WriteSealLog(4, "QtDemo::UpdateAlarm->读报警器控制状态失败");
        return;
    }

    ui.door_alarm_status_->setText(
        0 == door ? QString::fromLocal8Bit("关") : QString::fromLocal8Bit("开"));
    ui.door_alarm_status_->adjustSize();

    ui.vibration_alarm_status_->setText(
        0 == vibration ? QString::fromLocal8Bit("关") : QString::fromLocal8Bit("开"));
    ui.vibration_alarm_status_->adjustSize();

    palette.setColor(QPalette::Background, update_succ);
    ui.door_alarm_status_->setPalette(palette);
    ui.vibration_alarm_status_->setPalette(palette);
}

// 印章列表信息
void QtDemo::UpdateStampers()
{
    QLabel* stampers[] =
    {
        ui.slot1_,
        ui.slot2_,
        ui.slot3_,
        ui.slot4_,
        ui.slot5_,
        ui.slot6_
    };

    QPalette palette;
    for (int i = 1; i < 7; ++i) {
        palette.setColor(QPalette::Background, update_hint);
        stampers[i - 1]->setPalette(palette);

        char id[14] = { 0 };
        int ret = GetABCStamper(i, id, 13);
        if (0 != ret) {
            char buf[512] = { 0 };
            sprintf_s(buf, "读印章%d的农行电子标签失败", i);
            WriteSealLog(4, buf);
            continue;
        }

        stampers[i - 1]->setText(QString::fromStdString(id));
        stampers[i - 1]->adjustSize();
        palette.setColor(QPalette::Background, update_succ);
        stampers[i - 1]->setPalette(palette);
    }
}

// 查看故障
void QtDemo::UpdateError()
{
    QPalette palette;
    palette.setColor(QPalette::Background, update_hint);
    ui.err_code_->setPalette(palette);

    if (!ec_vec_.empty()) {
        ui.err_code_->setText(QString::fromLocal8Bit("有故障, 点击\"故障详情\""));
        ui.pb_error_des_->setEnabled(true);
        ui.pb_error_des_->setStyleSheet("background-color: rgb(255, 0, 0);");
    }

    palette.setColor(QPalette::Background, update_succ);
    ui.err_code_->setPalette(palette);
}

void QtDemo::UpdateCameras()
{
    QPalette palette;
    palette.setColor(QPalette::Background, update_hint);
    ui.paper_cam_status_->setPalette(palette);
    ui.env_cam_status_->setPalette(palette);
    ui.safe_cam_status_->setPalette(palette);

    if (open_called) {
        QString str = QString::fromLocal8Bit("在线");
        ui.paper_cam_status_->setText(str);
        ui.env_cam_status_->setText(str);
        ui.safe_cam_status_->setText(str);
    }

    palette.setColor(QPalette::Background, update_succ);
    ui.paper_cam_status_->setPalette(palette);
    ui.env_cam_status_->setPalette(palette);
    ui.safe_cam_status_->setPalette(palette);

    // 停止定时器
    timer_->stop();

    // 更新状态栏
    char msg[512] = { 0 };
    sprintf_s(msg, "检测完毕");
    ui.statusBar->showMessage(QString::fromLocal8Bit(msg), STATUS_TEXT);
}

// 定时器到期函数运行在主线程中
void QtDemo::TimerDone()
{
    count_mtx_.lock();
    int tmp_count = count_++;
    count_mtx_.unlock();

    char buf[512] = { 0 };
    sprintf_s(buf, "QtDemo::TimerDone->当前count_ = %d", tmp_count);
    WriteSealLog(4, buf);
    switch (tmp_count) {
    case 0:
        UpdateDeviceSN();
        break;
    case 1:
        UpdateDeviceStatus();
        break;
    case 2:
        UpdateDoors();
        break;
    case 3:
        UpdateAlarm();
        break;
    case 4:
        UpdateStampers();
        break;
    case 5:
        UpdateError();
        break;
    case 6:
        UpdateCameras();
        break;
    default:
        break;
    }

    // 需要加锁保护共享变量connected
    mtx.lock();
    if (connected == 0) {
        connected = -1;
        Info(QString::fromLocal8Bit("设备已断开"));
    } else if (connected == 1) {
        connected = -1;
        Info(QString::fromLocal8Bit("设备重连成功"));
    }
    mtx.unlock();
}

struct CusRGB {
    CusRGB(int r, int g, int b) : r_(r), g_(g), b_(b) {}

    int r_;
    int g_;
    int b_;
};

void QtDemo::UpdateRedetectTimer()
{
    static int idx = 0;
    static CusRGB rgbs[] =
    {
        CusRGB(131, 175, 155),
        CusRGB(200, 200, 169),
        CusRGB(249, 205, 173),
        CusRGB(252, 157, 154),
        CusRGB(254, 64,  101),
    };

    char buf[512] = { 0 };
    sprintf_s(buf, "background-color: rgb(%d, %d, %d);", 
        rgbs[idx].r_,
        rgbs[idx].g_,
        rgbs[idx].b_);
    int i = idx;
    if (++idx > 4)
        idx = 0;

    ui.pb_redetect_->setStyleSheet(buf);
    ui.pb_redetect_->setGeometry(
        ui.pb_redetect_->x(), 
        ui.pb_redetect_->y(), 
        81 + 5 * i, 
        61 + 5 * i);

//     srand((unsigned int)time(NULL));
//     int x_ran = rand() % 565;
//     int y_ran = rand() % 565;
//     ui.logo_img_->setGeometry(x_ran, y_ran, ui.logo_img_->width(), ui.logo_img_->height());
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

void QtDemo::HandleLogoClicked()
{
    ui.logo_img_->setDisabled(true);
    ui.logo_img_->hide();
}
