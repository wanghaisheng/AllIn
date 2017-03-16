#ifndef USBCONTROLF60_API_H
#define USBCONTROLF60_API_H

#include <string>

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 USBCONTROLF60_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// USBCONTROLF60_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef USBCONTROLF60_EXPORTS
#define USBCONTROLF60_API _declspec(dllexport)
#else
#define USBCONTROLF60_API _declspec(dllimport)
#endif


//设备打开失败，发送数据错误，，应答命令出错，升级文件错误，下位机校验出错
//升级数据发送失败，系统错误

// 函数返回值定义
#define STF_SUCCESS									((int)0)  //操作成功
#define STF_ERR_OPENDEVFAIL					        ((int)-1) //设备打开失败
#define STF_ERR_SENDFAIL						    ((int)-2) //发送数据错误
#define STF_ERR_RECVFAIL						    ((int)-3) //应答错误
#define STF_ERR_COMMUNICATION				        ((int)-4) //应答命令出错
#define STF_ERR_INVLIDATEBINFILE		            ((int)-5) //升级文件错误
#define STF_ERR_FIREWARE_FILECHECK	                ((int)-6) //下位机校验出错
#define STF_INVALID_PARAMETER                       ((int)-7) //参数非法
#define STF_ERR_INTERNEL						    ((int)-99) //升级系统错误
#define STF_STAMPER_LOSS                            ((int)-100)//缺章
#define STF_INVALID_STAMPER                         ((int)-101)//非本机绑定印章
#define STF_DUPLICATE_STAMPER                       ((int)-102)//印章ID重复
#define STF_STAMPER_SEQ_CHANGE                      ((int)-103)//印章顺序发生改变
#define STF_ALLOCATE_MEM_FAIL                       ((int)-104)//分配内存失败
#define STF_WRITE_MAC_FAIL                          ((int)-105)//保存MAC地址失败
#define STF_READ_MAC_FAIL                           ((int)-106)//读取MAC地址失败

#define OUT

//RFID版修改
typedef struct _stamperparam{
    unsigned int        serial_number;  //盖章流水号
	char				seal[4];
	char				isPadInk; 

	unsigned short		x_point; 
	unsigned short		y_point; 
	unsigned short		angle;
	unsigned short		w_time; 
    unsigned char       type;           //盖章方式(0---普通盖章, 1---骑缝章)

	_stamperparam()
	{
        this->serial_number = 0;
		this->seal[0]		= 0;
		this->seal[1]		= 0;
		this->seal[2]		= 0;
		this->seal[3]		= 0;
		this->seal[4]		= 0;
		this->isPadInk		= 0;
		this->x_point		= 0; 
		this->y_point		= 0;
		this->angle			= 0;
		this->w_time		= 0;
        this->type          = 0;
	}

	_stamperparam& operator= (_stamperparam &p)
	{
        this->serial_number     = p.serial_number;
		strcpy_s(this->seal, p.seal);
		this->isPadInk			= p.isPadInk;
		this->x_point			= p.x_point; 
		this->y_point			= p.y_point;
		this->angle				= p.angle;
		this->w_time			= p.w_time;
        this->type              = p.type;
		return *this;
	}

}STAMPERPARAM, *PSTAMPERPARAM;

//设备连接状态回调函数
//DevPath   设备路径
//uMsg		1 连接, 0 断开
typedef int ( _stdcall *pDevConnectCallBack)(const char* DevPath, unsigned int uMsg);

/*USB通信回调函数
	uMsg:
		0xA0:盖章过程中下压通知
		0xA1:盖章过程中机械手臂回到印油线，提示可以拍照
		0xA2:盖章完成通知，wParam 0 成功， 1 失败
		0xA3:盖章过程中 印章掉落通知
		0xA4:纸门关闭通知, (wParam为0表示门关闭通知)
        0xA5:盖章过程错误通知
        0xA6:侧门关闭通知, (wParam为0表示侧门关闭通知; 1表示侧门打开通知)
        0xA7:顶盖门关闭通知, (wParam为0表示顶盖关闭通知; 1表示顶盖打开通知)
        0xA8:电子锁上锁通知
*/
#define STAMPER_DOWN				        (unsigned char)0xA0
#define STAMPER_ARMBACK						(unsigned char)0xA1
#define STAMPER_COMPLETE					(unsigned char)0xA2
#define STAMPER_SEALERR						(unsigned char)0xA3
#define STAMPER_PAPERDOORCLOSE		        (unsigned char)0xA4
#define STAMPER_ERROR                       (unsigned char)0xA5
#define STAMPER_SIDEDOOR_CLOSE              (unsigned char)0xA6
#define STAMPER_TOP_CLOSE                   (unsigned char)0xA7
#define STAMPER_ELEC_LOCK                   (unsigned char)0xA8

//仅当uMsg==STAMPER_COMPLETE(盖章完成)时, data不为空, len为46, 按接口协议文档解析data,
//可以得到“流水号”, "印章ID号"等.
typedef int (_stdcall *pfnDevMsgCallBack)(unsigned int uMsg, unsigned int wParam, long lParam,
                                        unsigned char* data, unsigned char len);

#ifdef __cplusplus
extern "C"{
#endif

/** 
*  @brief    注册设备回调函数(设备重连与断开通知)
*  @details  
*  @param    [pDevConnectCallBack] func   回调函数
*  @return   =STF_SUCCESS
*  @remark
*/
USBCONTROLF60_API int F_RegisterDevCallBack(pDevConnectCallBack func);

/** 
*  @brief    日志记录
*  @details  
*  @param    level,日志等级
*            1--调试
*            2--提示
*            3--警告
*            4--错误
*  @param    [const char*] szLog   日志内容
*  @return   =STF_SUCCESS
*  @remark
*/
USBCONTROLF60_API void F_RecordLog(int level, const char* szLog);

 // 此类是从 USBControlF60.dll 导出的
 class USBCONTROLF60_API CUSBControlF60 {
 public:
	 CUSBControlF60(void);
	 ~CUSBControlF60(void);
 };

 extern USBCONTROLF60_API int nUSBControlF60;
 USBCONTROLF60_API int fnUSBControlF60(void);

 /** 
 *  @brief    注册回调函数(盖章结果, 纸门关闭通知)
 *  @details  
 *  @param    [pfnDevMsgCallBack] pfn   回调函数地址
 *  @return   =STF_SUCCESS
 *  @remark
 */
USBCONTROLF60_API int FRegisterDevCallBack(pfnDevMsgCallBack pfn);

//打开设备, 参数无效
USBCONTROLF60_API	int FOpenDev(const char *file);

//关闭设备
USBCONTROLF60_API	int FCloseDev(void);

//0x01, 退出维护模式
USBCONTROLF60_API	int FQuitMaintainMode(void);

//0x02, 重启印控机主板
USBCONTROLF60_API int FRestart(void);

//0x03, 获取系统信息, 参照协议
USBCONTROLF60_API int FGetSystemMsg(char *pRtn, int nLen);

//0x04, 获取设备固件版本号(strVer, len=8)
USBCONTROLF60_API	int FGetFirwareVer(OUT char* strVer, int len);

//0x05, 设置设备序列号(len=14)
USBCONTROLF60_API	int FSetFirwareSN(char* strSN, int len);

//0x06, 获取设备序列号(len=15)
//len       --- 至少15个字节空间
USBCONTROLF60_API	int FGetFirwareSN(OUT char* strSN, int len);

//0x07, 查询设备状态(len=14, 详细见协议)
USBCONTROLF60_API	int FGetDevStatus(OUT unsigned char* strStatus, int len);

//0x08, 进入维护模式
USBCONTROLF60_API	int FMaintenanceMode(void);

//0x09, 获取红外状态,len见协议
USBCONTROLF60_API	int FGetInfraRedStatus(OUT char* strInfraRed, int len);

//0x0A, 获取印章状态(len=30)
USBCONTROLF60_API int FGetSealPresent(OUT char* strSealStatus, int len);

//0x0B, 获取门状态(len = 4)
//P1:   推纸门状态  0 关闭，1 开启， 2检测错误
//P2:   电子锁状态，同上
//P3:   机械锁状态，同上
//P4:   顶盖状态，同上
USBCONTROLF60_API int FGetDoorsPresent(OUT char* strDoorsStatus, int len);

//0x0C, 打开推纸门
USBCONTROLF60_API	int FOpenDoorPaper();

//0x0D, 打开安全门
USBCONTROLF60_API	int FOpenDoorSafe(void);

//0x0E, 关闭安全门
USBCONTROLF60_API	int FCloseDoorSafe(void);

//0x0F, 补光灯控制
//light     --- 补光灯类型
//              1 -- 安全门旁边的补光灯; 
//              2 -- 凭证摄像头旁边的补光灯
//op        --- 操作(0 -- 关; 1 -- 开)
USBCONTROLF60_API	int FLightCtrl(char light, int op);

//0x10, X移动, 需要在测试模式下
USBCONTROLF60_API	int FMoveX(unsigned short x_point);

//0x11, Y移动
USBCONTROLF60_API	int FMoveY(unsigned short y_point);

//0x12, 转章
USBCONTROLF60_API	int FTurnSeal(unsigned short angle);

//0x13, 普通盖章
USBCONTROLF60_API	int FStartStamperstrc(STAMPERPARAM *pstamperparam);

//盖章
USBCONTROLF60_API	int FStartStamper(
    unsigned int serial,
    char *seal, 
    char isPadInk,
    unsigned short x_point, 
    unsigned short y_point,
    unsigned short angle, 
    unsigned short w_time,
    unsigned char type = 0);

//农行接口盖章
//serial    --- 用印流水号
//id        --- 印章电子标签(STDZ + 8位编号)
//isPadInk  --- 是否使用印油
//x_point   --- 用印X坐标
//y_point   --- 用印Y坐标
//angle     --- 用印角度
//w_time    --- 用印时间
//type      --- 盖章类型(0---默认普通章, 1---骑缝章)
USBCONTROLF60_API int FStartStamperABC(
    unsigned int serial,
    char* abc_id,
    char isPadInk,
    short x_point,
    short y_point,
    short angle,
    short w_time,
    unsigned char type = 0);

//用印
//serial    --- 用印流水号
//sealNo    --- 印章编号[1-6]
//isPadInk  --- 是否使用印油
//x_point   --- 用印X坐标
//y_point   --- 用印Y坐标
//angle     --- 用印角度
//w_time    --- 用印时间
//type      --- 盖章类型(0---默认普通章, 1---骑缝章)
USBCONTROLF60_API int FStartStamperEx(
    unsigned int serial,
    char sealNo, 
    char isPadInk,
    short x_point, 
    short y_point,
    short angle, 
    short w_time,
    unsigned char type = 0);

//0x14, 选章和是否沾印油
//serial    --- 用印流水号
//seal_id   --- 印章ID
//isPadInk  --- 是否蘸印油
USBCONTROLF60_API int FSelectStamper(
    unsigned int serial, 
    unsigned int seal_id, 
    char isPadInk);

//0x15, 取消盖章
USBCONTROLF60_API int FCancleStamper(void);

//0x16, 蜂鸣器控制
//beep      --- 0 关闭; 1 长鸣; 2 间隔响
//interval  --- 当beep=2时该值有效, 间隔响时常(单位秒)
USBCONTROLF60_API	int FBeepCtrl(char beep, char interval);

//0x17, 补光灯亮度调节
//light         --- 补光灯类型
//                  1 安全门旁边的补光灯
//                  2 凭证摄像头旁边的补光灯
//brightness    --- 亮度值(建议范围 1-100, 1为最弱, 100为最亮)
USBCONTROLF60_API	int FLightBrightness(char light, char brightness);

//0x18, 印章归位
USBCONTROLF60_API	int FSealBack(void);

//0x19, 进入功能测试模式
USBCONTROLF60_API	int FInTestMode(void);

//0x1A, 退出功能测试模式, 设备会自动重启
USBCONTROLF60_API	int FOutTestMode(void);

//0x1B, 获取Mac地址
//num       --- 1(获取第一个MAC地址),
//              2(第二个MAC地址)
//strmac    --- 返回具体的MAC地址
//len       --- 存放MAC地址长度, MAC地址最大18个字节
USBCONTROLF60_API int GetMacAdress(char num, OUT char* strmac, int len);

//0x1C, 绑定Mac地址
//op        --- 1(绑定MAC)
//              0(解绑MAC)
//num       --- 表示操作哪个MAC(1, 2)
//strmac    --- MAC地址串
USBCONTROLF60_API	int FBindMac(char op, char num, char* strmac, int len);

//0x1D, 设置报警功能
//alarm     --- 0(开门报警器)
//              1(振动报警器)
//switchs   --- 报警器开关
//              1(开启);
//              0(关闭)
USBCONTROLF60_API int SetAlarm(char alarm, char operation);

//0x1E, 设置设备认证码
//最多支持设置10个字节的认证码
USBCONTROLF60_API int SetDevCode(char* code, int len);

//0x1F, 获取设备认证码
//认证码最多有10个字节
USBCONTROLF60_API int GetDevCode(OUT char* code, int len);

//0x20, 保存数据到印章机存储器中(最多支持512字节)
//如果调用该接口请从偏移量150以后开始写.
//
//offset    --- 写偏移量
//data      --- 待写入数据
//len       --- 待写入数据长度
USBCONTROLF60_API int WriteIntoStamper(
    unsigned short offset,
    const unsigned char* data,
    unsigned char len);

//0x21, 读取印章机内的存储器数据
//如果调用该接口请从偏移量150以后开始读.
//
//offset    --- 读偏移量
//size      --- 请求读数据长度
//data      --- 存放读取到的数据
//len       --- 实际返回数据长度
USBCONTROLF60_API int ReadStamperMem(
    unsigned short offset,
    unsigned short size,
    unsigned char* data,
    unsigned char& len);

//0x22, 存储器版本号及可用空间大小
//
//version   --- 存储器版本号
//memz_size --- 存储器可用空间
USBCONTROLF60_API int GetStorageCapacity(unsigned char& version, unsigned short& mem_size);

//0x23, 进入认证状态, 目前默认返回-1(失败)
USBCONTROLF60_API int EnterAuthMode();

//0x24, 退出认证状态, 目前默认返回-1(失败)
USBCONTROLF60_API int ExitAuthMode();

//0x25, 选取印章
//该接口供读写印章RFID时调用
//
//stamper  --- 印章仓位号,从0开始
USBCONTROLF60_API int SelectStamper(unsigned char stamper);

//0x26, 根据印章仓位号获取对应的RFID号
//
//stamper   --- 印章仓位号, (下标从0开始)
//rfid      --- 对应的rfid号
USBCONTROLF60_API int GetStamperID(unsigned char stamper, unsigned int& rfid);

//根据印章仓位号获取按农行指定规则的印章电子标签(STDZ + 8位长度编号)
//
//stamper   --- 印章仓位号, 从1开始
//id        --- 农行印章电子标签, STDZ00000001, 12个字节
//len       --- 农行电子标签长度, 至少13个字节
//
//返回值:
//      0   --- 成功
//      1   --- 获取农行印章电子标签失败
//      2   --- 指定印章号未按农行要求写入过印章编号
USBCONTROLF60_API int GetABCStamper(unsigned char stamper, char* id, unsigned char len);

//根据农行指定规则的印章电子标签获取印章仓位号
//
//id        --- 农行电子标签, 如'STDZ00000002'
//index     --- 印章仓位号, 从1开始
//
//返回值:
//      0   --- 成功
//      1   --- 失败
USBCONTROLF60_API int GetABCStamperIndex(const char* abc_id, char* index);

//0x27, 操作指定块的绝对, 操作块时要按照以下流程:
//1. 卡选择(SelectStamper),
//2. 卡请求(GetStamperID),
//3. 设置地址(OperateBlock),
//4. 密码比较(VerifyKey),
//5. 卡读写(WriteBlock, ReadBlock)
USBCONTROLF60_API int OperateBlock(unsigned char block);

//0x28, 验证密钥
//
//key_type  --- 密钥类型(A、B密码)
//              0对应A密码
//              1对应B密码
//key       --- key_type对应的密钥
//len       --- 密钥数据长度(6字节)
USBCONTROLF60_API int VerifyKey(char key_type, const unsigned char* key, unsigned char len = 6);

//0x29, 读指定块
//block     --- 块号
//data      --- 读取成功后保存读取的数据
//len       --- 数据长度(应大于17个字节)
USBCONTROLF60_API int ReadBlock(unsigned char block, unsigned char* data, unsigned char len);

//0x2A, 写指定块
//block     --- 块号
//data      --- 待写入数据
//len       --- 数据长度(最多支持写17个字节)
USBCONTROLF60_API int WriteBlock(unsigned char block, unsigned char* data, unsigned char len);

//0x2B, 获取物理盖章范围, 具体解析见协议
USBCONTROLF60_API int   GetPhsicalRange(char * phsicalOut, int len);

//0x2C, 设置印章映射关系
USBCONTROLF60_API int   SetStampMap();

//0x2D, 获取印章映射关系
//len       --- 数据长度, 52字节, 每个章槽占用8个字节(前4个字节为章ID, 后4个字节为保留信息),
//              最后4字节为32位整型和校验码取反值
USBCONTROLF60_API int   GetStampMap(char* mapout, int len);

//0x30, 读取当前所有RFID, 包括侧门, 共7个rfid, rfid值为四字节无符号整型
//rfids     --- 存放rfid值, 元素值为0表示无章, 非0为对应的RFID号
//len       --- rfid个数, 共有7个rfid(第7个为侧门rfid)
//stampers  --- 返回实际的rfid章个数
USBCONTROLF60_API int ReadAllRFID(unsigned int* rfids, unsigned char len, unsigned char* stampers);

//0x31, 读报警器电池电压值
//
//返回值为0时电压值有效, 单位为0.1伏
USBCONTROLF60_API int ReadAlarmVoltage(unsigned char* voltage);

//0x32, 统计信息清零
USBCONTROLF60_API int ClearStatistic();

//0x33, 复位, USB不会断开
USBCONTROLF60_API int Reset();

//0x34, 确认盖章
//先选章, 再确认盖章
//先调用接口GetStamperID由印章号得到印章RFID, 再调用接口FSelectStamper完成选章, 最后
//调用该接口Confirm实现盖章确认完成盖章.
//
//serial    --- 流水号,该流水号与接口FSelectStamper传入的流水号必须一致
//rfid      --- 印章ID号,调用接口GetStamperID获得
USBCONTROLF60_API int Confirm(
    unsigned int serial,
    unsigned int rfid,
    char isPadInk,
    short x_point,
    short y_point,
    short angle,
    short w_time,
    unsigned char type = 0);

//0x35, 读盖章信息, 详细数据格式见协议
USBCONTROLF60_API int ReadStamp(unsigned char* info, unsigned char len = 47);

//0x36, 设置侧门开门提示信息
//keep_open     --- 打开侧门后锁保持打开的时间, 单位秒
//timeout       --- 指定时间内未关闭侧门, 蜂鸣器提示, 单位秒
USBCONTROLF60_API int SetSideDoor(unsigned short keep_open, unsigned short timeout);

//0x37, 设置推纸门开启后超时提示时间
//timeout   --- 超时未关门提示,默认30秒
USBCONTROLF60_API int SetPaperDoor(unsigned short timeout = 30);

//0x38, 读取报警器控制状态(包括振动报警和门报警)
//
//door          --- 门报警, 0是关, 1是开
//vibration     --- 振动报警, 0是关, 1是开
//
//返回值:
//      0: 读报警器状态成功
//      1: 失败
//      2: 忙
USBCONTROLF60_API int ReadAlarmStatus(char* door, char* vibration);

//0x39, 读取硬件版本号字符串, 与实际版本代码做关联如"master_2.41"
//version       --- 版本号字符串
//len           --- 最长版本号长度(255个字节)
USBCONTROLF60_API int GetHardwareVer(char* version, unsigned char len = 255);

//0x51, 工厂测试模式, 开启后不需要电脑也可循环盖章测试
//enable    --- 1(开启工厂模式), 0(关闭工厂模式)
USBCONTROLF60_API int EnableFactoryMode(unsigned char enable);

//0x52, USB调试信息输出开关
//op    --- 1 开; 0 关
USBCONTROLF60_API int FDebugLogSwitch(char op);

//(0xB1~0xB5) 升级数据接口
// 命令，数据，数据长度
USBCONTROLF60_API int FfirewareUpdate(unsigned char cmd, unsigned char* data, int len);

//0xD0, 读备用板序列号, 目前主板也可以使用该命令
USBCONTROLF60_API int ReadBackupSN(unsigned char* sn, unsigned char len = 48);

//0xD1, 写备用板序列号, 目前主板也可以使用该命令
USBCONTROLF60_API int WriteBackupSN(const unsigned char* sn, unsigned char len = 48);

//印章校准 
USBCONTROLF60_API  int Calibration( char* points, int len);

USBCONTROLF60_API int  CalculatePos(double* x1, double* y1, double* x2, double* y2, 
    double* scalex,double*scaley);

USBCONTROLF60_API int  CalculatePosition(int stamperPointX,int stanperPointY,char* points,int len);

USBCONTROLF60_API  int  CalibrationMutiple(char * points,int len);

//印章校准农行 
//pStampid      --- 印章ID
USBCONTROLF60_API int CalibrationEx(char * pStampid, char *points, int len);

//存储MAC地址, 不包括结尾字符('\0')
//MAC地址如: "30-3A-64-D6-FD-30"
//支持直接写入(占用17个字节), 或者转换为十进制写入"48-58-100-214-253-48"(占用11个字节)
USBCONTROLF60_API int WriteMAC(
    const unsigned char* mac1, 
    const unsigned char* mac2, 
    unsigned char max_mac1_len = 17,
    unsigned char max_mac2_len = 17);

//读取MAC地址
//mac1,mac2     --- 存放读取到的对应MAC地址
//max_len       --- 数组长度, 至少为18(MAC地址最大支持17个字符, 返回自动添加一个结尾字符)
USBCONTROLF60_API int ReadMAC(
    unsigned char* mac1, 
    unsigned char* mac2, 
    unsigned char max_len = 18);

//存储设备key值
USBCONTROLF60_API int WriteKey(const unsigned char* key, unsigned char key_len = 16);

//读取设备Key
//key_len       --- 至少17个字节空间
USBCONTROLF60_API int ReadKey(unsigned char* key, unsigned char key_len = 17);

//存储图像转换倍率
//x,y坐标, 8字节浮点数
USBCONTROLF60_API int WriteImageConvRatio(float* x, float* y);

//读取图像转换倍率
USBCONTROLF60_API int ReadImageConvRatio(float* x, float* y);

//根据印章RFID号获取对应的仓位号
//
//rfid      --- rfid号
//stamper   --- rfid对应的印章仓位号, 从1开始
USBCONTROLF60_API int GetStamper(unsigned int rfid, unsigned char& stamper);

//存储校准点数据
//根据硬件尺寸大小,每个校准点的x,y值用2个字节存放即可, 有5个校准点.
//points    --- 顺序存放每个点的x,y值,即x1,y1,x2,y2...x5,y5
//len       --- 数据长度
USBCONTROLF60_API int WriteCalibrationPoint(unsigned short* points, unsigned char len = 10);

//获取校准点数据
//数据格式参见存储校准点数据接口
USBCONTROLF60_API int CalibrationPoint(unsigned short* points, unsigned char len = 10);

//检查印章信息
//将当前印章id与设备存储的印章id信息进行比对，
//根据返回值可以知道当前设备是否可用,及不可用的具体原因,上层决定处理的策略
//返回值:
//      STF_STAMPER_LOSS        //缺章
//      STF_INVALID_STAMPER     //出现非本机绑定印章
//      STF_DUPLICATE_STAMPER   //印章ID重复
//      STF_STAMPER_SEQ_CHANGE  //印章顺序发生改变,不用特殊处理
USBCONTROLF60_API int CheckStampers();

//印控仪锁定状态
USBCONTROLF60_API bool IsLocked();

//锁定印控仪
USBCONTROLF60_API int Lock();

//解锁印控仪
USBCONTROLF60_API int Unlock();

//写印控仪编号
//id        --- 印控仪编号
//len       --- 编号长度, 最多支持20个字节
USBCONTROLF60_API int WriteStamperIdentifier(const unsigned char* id, unsigned char len);

//读取印控仪编号
//id        --- 保存印控仪编号
//len       --- 最小要有20个字节空间
USBCONTROLF60_API int ReadStamperIdentifier(unsigned char* id, unsigned char len);

#ifdef __cplusplus
 }
#endif

#endif
