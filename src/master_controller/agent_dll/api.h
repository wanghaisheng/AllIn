#ifndef MC_AGENT_API_H_
#define MC_AGENT_API_H_

#ifdef MASTERCTRL_AGENT_EXPORTS
#define MASTERCTRL_AGENT_API _declspec(dllexport)
#else
#define MASTERCTRL_AGENT_API _declspec(dllimport)
#ifdef _DEBUG
#pragma comment(lib, "agentd.lib")
#else
#pragma comment(lib, "agent.lib")
#endif
#endif

#ifdef __cplusplus
extern "C"{
#endif

// 设备连接状态回调函数
// msg:	    1 --- 连接
//          0 --- 断开
typedef int (_stdcall *ConnectCallback)(
    unsigned int msg);

// USB通信回调函数
// msg:
//      0xA0:    盖章过程中下压通知
//      0xA1:    盖章过程中机械手臂回到印油线, 提示可以拍照
//      0xA2:    盖章完成通知, param = 0 成功; 1 失败
//      0xA3:    盖章过程中, 印章掉落通知
//      0xA4:    纸门关闭通知, (param为0表示门关闭通知)
//      0xA5:    盖章过程错误通知
//      0xA6:    侧门关闭通知, (param为0表示侧门关闭通知; 1表示侧门打开通知)
//      0xA7:    顶盖门关闭通知, (param为0 表示顶盖关闭通知; 1 表示顶盖打开通知)
//      0xA8:    电子锁上锁通知
// 
typedef int (_stdcall *EventCallback)(
    unsigned int msg,
    unsigned int param);

MASTERCTRL_AGENT_API int RegisterConnCallBack(ConnectCallback func);

MASTERCTRL_AGENT_API int RegisterEventCallBack(EventCallback func);


// 该接口文件包含以下4类接口：
// 1. 印控机接口;
// 2. 图像处理接口;
// 3. 摄像头接口;
// 4. 查询错误信息接口;

/////////////////////// 1. 印控机API //////////////////////////////

// 获取系统信息
// status:
//      0 主系统
//      1 备用系统
//      2 主系统升级模式
//      3 备用系统升级模式
MASTERCTRL_AGENT_API int ST_GetSystemInfo(
    int& status);

// 获取印控机与PC连接状态
// cnn      --- 0: 断开连接
//          --- 1: 已连接
// 返回值：
//          --- 0：成功
//          --- 7：连接线已断开(印控机与PC未线连)
MASTERCTRL_AGENT_API int ST_QueryCnn(
    int& cnn);

// 打开设备连接
MASTERCTRL_AGENT_API int ST_Open();

// 关闭设备连接
MASTERCTRL_AGENT_API int ST_Close();

// 获取设备状态
// 0 ---- "未初始化"
// 1 ---- "启动自检"
// 2 ---  "检测章"
// 3 ---- "空闲状态"
// 4 ---- "测试模式"
// 5 ---- "故障模式"
// 6 ---- "盖章模式"
// 7 ---- "维护模式"
MASTERCTRL_AGENT_API int ST_GetDevStatus(
    int& code);

// 获取印控仪编号(只针对主板)
MASTERCTRL_AGENT_API int ST_QueryMachine(
    char* sn,
    int size = 24);

// 设置印控机编号(只针对主板), 最多支持20个字节
MASTERCTRL_AGENT_API int ST_SetMachine(
    const char* sn);

// 查询MAC地址, 一台印控机最多支持绑定2个MAC地址
// mac1         --- mac地址1
// mac2         --- mac地址2
MASTERCTRL_AGENT_API int ST_QueryMAC(
    char* mac1, 
    char* mac2,
    int max_size = 18);

// 绑定MAC地址
// mac     --- 待绑定MAC地址
MASTERCTRL_AGENT_API int ST_BindMAC(
    const char* mac);

// 解绑MAC地址
// mac     --- 待解绑MAC地址
MASTERCTRL_AGENT_API int ST_UnbindMAC(
    const char* mac);

// 根据卡槽号获取对应的RFID
// slot         --- 卡槽号，从1开始
// rfid         --- 对应卡槽号的rfid
MASTERCTRL_AGENT_API int ST_GetRFID(
    int     slot,
    int&    rfid);

// 准备用印
// stamp_num        --- 章槽号, 从1开始(deprecated)
// timeout          --- 进纸门超时未关闭时间, 单位秒
//                      建议设置30秒以上, 当传入0时使用默认值
// task_id          --- 任务号, 仅能使用一次
MASTERCTRL_AGENT_API int ST_PrepareStamp(
    char            slot,
    int             timeout, 
    char*           task_id,
    int             size = 64);

// 普通用印
MASTERCTRL_AGENT_API int ST_OrdinaryStamp(
    const char*         task,       // 任务号
    int                 slot,       // 印章卡槽号(1-6)
    int                 ink,        // 0 - 不蘸印油， 1 - 蘸印油
    int                 x,          // 盖章位置x坐标, 相对于印控机坐标系
    int                 y,          // 盖章位置y坐标, 相对于印控机坐标系
    int                 angle,      // 印章旋转角度, 大于等于0且小于360度
    int                 type = 0);  // 0 - 普通用印, 1 - 骑缝章

// 结束用印. 完成以下操作：
// 1. 解锁印控仪;
// 2. 删除任务号;
// 3. 弹出纸门;
// 
// task         --- 准备用印时获得的用印任务号
MASTERCTRL_AGENT_API int ST_FinishStamp(
    const char* task);

// 查询印章状态
// status   --- 对应章槽上是否有章
//              0 - 表示无章, 1 - 表示有章. 如"001101\0"
// len      --- 数组长度（包括结尾0字符）
MASTERCTRL_AGENT_API int ST_QueryStampers(
    char* status, int len = 7);

// 查询进纸门状态
// status        --- 0-关, 1-开
MASTERCTRL_AGENT_API int ST_QueryPaper(
    int& status);

// 打开进纸门
// timeout      --- 纸门超时未关闭时间，单位秒,
//                  传入0时使用默认值
MASTERCTRL_AGENT_API int ST_OpenPaper(
    int timeout = 30);

// 安全门状态查询
// status   --- 0-关,1-开
MASTERCTRL_AGENT_API int ST_QuerySafe(
    int& status);

// 开关安全门
// ctrl     --- 0 - 关, 1 - 开
MASTERCTRL_AGENT_API int ST_ControlSafe(
    int ctrl);

// 顶盖门状态查询
// status        --- 0-关, 1-开
MASTERCTRL_AGENT_API int ST_QueryTop(
    int& status);

// 进入维护模式
MASTERCTRL_AGENT_API int ST_EnterMaintain();

// 退出维护模式
MASTERCTRL_AGENT_API int ST_ExitMaintain();

// 蜂鸣器开关
// ctrl     --- 0 - 关, 1 - 开
// type     --- 0 - 长鸣，1 - 间隔响
// interval --- 当type = 1时该参数有效, 间隔响时常(单位秒)
MASTERCTRL_AGENT_API int ST_ControlBeep(
    int ctrl,
    int type = 0,
    int interval = 2);

// 读取报警器控制状态(包括振动报警和门报警)
//
// door          --- 门报警, 0 - 关, 1 - 开
// vibration     --- 振动报警, 0 - 关, 1 - 开
MASTERCTRL_AGENT_API int ST_ReadAlarm(
    int& door, 
    int& vibration);

// 报警器开关
//alarm     --- 0(开门报警器)
//              1(振动报警器)
//switches  --- 报警器开关
//              1(开启);
//              0(关闭)
MASTERCTRL_AGENT_API int ST_ControlAlarm(
    int alarm, 
    int switches);

// 卡槽数量查询
// num      --- 实际印章数
MASTERCTRL_AGENT_API int ST_QuerySlots(
    int &num);

// 锁定印控仪
MASTERCTRL_AGENT_API int ST_Lock();

// 解锁印控仪
MASTERCTRL_AGENT_API int ST_Unlock();

// 印控仪锁定状态
// lock     --- 0: 已锁定
//              1：未锁定
MASTERCTRL_AGENT_API int ST_QueryLock(
    int& lock);

// 设置安全门
MASTERCTRL_AGENT_API int ST_SetSideDoor(
    int keep,
    int timeout);

//  获取设备型号
MASTERCTRL_AGENT_API int ST_GetDevModel(
    char* model,
    int size = 22);

// 补光灯控制
// which    --- 补光灯类型
//              1 -- 安全门的补光灯; 
//              2 -- 凭证摄像头的补光灯
// ctrl     --- 控制开关
//              0 -- 关; 1 -- 开
// value    --- 打开补光灯同时设置补光灯的亮度值,
//              1-100, 1为最弱, 100为最亮。
MASTERCTRL_AGENT_API int ST_ControlLed(
    int which,
    int ctrl,
    int value = 23);

// 用印参数检查(针对印控机的物理坐标)
// x        --- x坐标值
// y        --- y坐标值
// angle    --- 章旋转角度值(0 ~ 360)
MASTERCTRL_AGENT_API int ST_CheckParam(
    int x,
    int y,
    int angle);

// 写图像转换倍率
// x, y坐标
MASTERCTRL_AGENT_API int ST_WriteImageConvRatio(
    float x, 
    float y);

// 读取图像转换倍率
MASTERCTRL_AGENT_API int ST_ReadImageConvRatio(
    float& x, 
    float& y);

// 存储校准点数据
// 根据硬件尺寸大小,每个校准点的x,y值用2个字节存放即可, 有5个校准点.
// points    --- 顺序存放每个点的x,y值,即x1,y1,x2,y2...x5,y5
// len       --- 数据长度
MASTERCTRL_AGENT_API int ST_WriteCalibrationPoint(
    unsigned short* points,
    unsigned char len = 10);

// 获取校准点数据
MASTERCTRL_AGENT_API int ST_ReadCalibrationPoint(
    unsigned short* points, 
    unsigned char len = 10);

// 开启工厂模式
MASTERCTRL_AGENT_API int ST_EnableFactory();

// 关闭工厂模式
MASTERCTRL_AGENT_API int ST_DisableFactory();

// 复位印控机, 不会断开连接
MASTERCTRL_AGENT_API int ST_Reset();

// 重启主板
MASTERCTRL_AGENT_API int ST_Restart();

// 读主板/备板序列号
MASTERCTRL_AGENT_API int ST_ReadMainStandbySN(
    char*       sn, 
    int         len = 48);

// 写主板/备板序列号
MASTERCTRL_AGENT_API int ST_WriteMainStandbySN(
    const char*     sn, 
    int             len = 48);

/////////////////////// 2. 图像处理API //////////////////////////////

// 合成照片
// p1       --- 图片1路径
// p2       --- 图片2路径
// merged   --- 合成图片路径
MASTERCTRL_AGENT_API int ST_MergePhoto(
    const char* p1, 
    const char* p2,
    const char* merged);

// 版面、验证码识别
// path         --- 切图路径
// model_type   --- 模板名称
// trace_num    --- 追溯码
MASTERCTRL_AGENT_API int ST_RecognizeImage(
    const char*  path, 
    char*        model_type, 
    char*        trace_num,
    int          max_size = 256);

// 要素识别
// path         --- 切图路径
MASTERCTRL_AGENT_API int ST_IdentifyElement(
    const           char* path, 
    int             x, 
    int             y, 
    int             width, 
    int             height,
    char*           result,
    int             size = 256);

// 原图用印位置、用印角度查找
MASTERCTRL_AGENT_API int ST_SearchSrcImageStampPoint(
    const char*     src_img_name,
    int             in_x,
    int             in_y,
    double          in_angle,
    int             &out_x,
    int             &out_y,
    double          &out_angle);

// 文件名中查找模板类型、用印角度、用印坐标
MASTERCTRL_AGENT_API int ST_RecoModelTypeAndAngleAndModelPointByImg(
    const char*     src_img,
    char*           model_type,
    int             model_type_size,
    double&         outangle,
    int&            x,
    int&            y);

// 二维码识别
// file         --- 待识别二维码图片, JPG格式
// qr           --- 识别结果
MASTERCTRL_AGENT_API int ST_RecognizeQRCode(
    const char* file,
    char* qr,
    const int qr_size = 32);

// 根据图片框选区域进行二维码识别
// file         --- 待识别二维码图片, JPG格式
// qr           --- 识别结果
MASTERCTRL_AGENT_API int ST_RecognizeQRCodeByRect(
    const char* file,
    const int left,
    const int top,
    const int right,
    const int bottom,
    char* qr,
    const int qr_size = 32);

/////////////////////// 3. 摄像头API //////////////////////////////

// 打开摄像头
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
MASTERCTRL_AGENT_API int ST_OpenCamera(
    int which);

// 关闭摄像头
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
MASTERCTRL_AGENT_API int ST_CloseCamera(
     int which);

// 查看摄像头状态
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
// status   --- 摄像头开关状态
//              0--关，1--开
MASTERCTRL_AGENT_API int ST_QueryCamera(
    int which, 
    int& status);

// 拍照
// which        --- 0：凭证摄像头
//                  1：环境摄像头
//                  2：侧门摄像头
// ori_path     --- 存放原图路径
// cut_path     --- 存放切图路径
MASTERCTRL_AGENT_API int ST_Snapshot(
    int                 which, 
    const char*         ori_path, 
    const char*         cut_path);

// 设置摄像头分辨率
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
// x        --- 宽度
// y        --- 高度
MASTERCTRL_AGENT_API int ST_SetResolution(
     int which,
     int x,
     int y);

// 设置JPG图片的DPI值
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
// dpi_x    --- 宽的DPI值
// dpi_y    --- 高的DPI值
MASTERCTRL_AGENT_API int ST_SetDPIValue(
    int which,
    int dpi_x, 
    int dpi_y);

MASTERCTRL_AGENT_API int ST_StartPreview(
    int which,
    int width,
    int height,
    int hwnd);

MASTERCTRL_AGENT_API int ST_StopPreview(
    int which);

// 开始录制视频
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
// path     --- 视频存放路径, 包含扩展名的全路径(.avi格式)
MASTERCTRL_AGENT_API int ST_StartRecordVideo(
    int which, 
    const char* path);

// 停止录制视频
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
// path     --- 视频存放路径, 包含扩展名的全路径(.avi格式)
MASTERCTRL_AGENT_API int ST_StopRecordVideo(
    int which,
    const char* path);

/////////////////////// 4. 查询错误信息 API //////////////////////////////

// 根据错误码获取错误信息
MASTERCTRL_AGENT_API int ST_GetError(
    int             err_code,
    char*           err_msg,
    char*           err_resolver,
    int             size = MAX_PATH);

#ifdef __cplusplus
}
#endif

#endif // MC_AGENT_API_H_
