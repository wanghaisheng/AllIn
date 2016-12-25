#ifndef _PSBC_STAMP_H
#define _PSBC_STAMP_H
#include <map>
#include <string>
using namespace std;




typedef int (*pinitializationMachine)(void);

/** No.2
* 函数:	querySloatInfo	
* 功能:	获取印章信息
* 说明:	0 表示无章,非零表示印章号.
*
* 参数:	
* @parm const char* machineNum 用印机编号
* @return	失败:返回失败码
*				成功:返回字符串 格式: 槽位号:印章ID;槽位号:印章ID(0 表示无章)... 
*				例(1:00000000001a;2:00000000002b;3:0...)
*/
typedef  char * (* pquerySealInfo)(const char* machineNum);

/** No.3
* 函数:	querySloatInfo	
* 功能:	获取卡槽信息
* 说明:	0 表示无章,1 表示有章
*
* 参数:	
* @parm const char* machineNum 用印机编号
* @return	失败:返回失败码
*				成功:返回字符串 格式: 槽位号:印章是否存在;槽位号:印章是否存在 (0 表示无章,1 表示有章)...
*				 例(1:0;2:1;3:0...)
*/
typedef char* (*pquerySlotInfo)(const char* machineNum);

/** No.4
* 函数:	initStamp	
* 功能:	设置印章信息
* 说明:	slotNumAndStampId长度不大于15*卡槽数量
*			
*
* 参数:	
* @parm const char* machineNum 用印机编号
* @parm const char* slotNumAndStampId 槽位号与印章ID循环串
*		例(1:00000000001a;2:00000000002b;3:0...)
* @return	成功: 0
*				失败:-1
*/
typedef int (*pinitStamp)(const char* machineNum,const char* slotNumAndStampId);

/** No.5
* 函数:	setAcrossPageSeal	
* 功能:	设置本次盖骑缝章,本次有效
* 说明:	设置本次盖章的类型是骑缝章,只有本次设定有效
*			
*
* 参数:	
* @parm void
* @return	成功: 0
*				失败:非0
*/
typedef int (*psetAcrossPageSeal)(void);

/** No.6
* 函数:	setMachineNum	
* 功能:	设置设备编号
* 说明:	设备出厂编号可自行修改,如需修改可在调用此接口时传入20位以内的字符串作为出厂编号
*
* 参数:	
* @parm const char* machineNum 设备编号(不大于12 位)
* @return	成功: 0
*				失败:非0
*/
typedef int (*psetMachineNum)(const char* machineNum);

/** No.7
* 函数:	openPaperDoor	
* 功能:	打开纸板
* 说明:	打开设备纸板以便放入凭证
*
* 参数:	
* @parm void
* @return	成功: 0
*				失败:非0
*/
typedef int (*popenPaperDoor)(void);

/** No.8
* 函数:	checkManualPrintPara	
* 功能:	用印—手工用印参数合法性判定
* 说明:	手工用印的盖章坐标点为印章的中心点基于图片左上角,以像素为单位
*			本函数判定手工用印传参是否在纸张有效范围内,适用于手工用印及光敏章用印
*			
*
* 参数:	
* @parm int pointX 用印X坐标,单位为像素
* @parm int pointY 用印Y坐标,单位为像素
* @parm int angle  转章角度(0-360)顺时针
* @return	合法: 0
*				非法:-1
*/
typedef int (*pcheckManualPrintPara)( int pointX, int pointY, int angle);

/** No.9
* 函数:	manualPrintStart	
* 功能:	用印—手工
* 说明:	转换后的盖章坐标点通过函数生成
*			手工用印采用剪裁后jpg图像的盖章坐标点为印章的中心点基于图片左上角,以像素为单位
*			
*
* 参数:	
* @parm int printNum 卡槽号
* @parm int pointX 用印X坐标,单位为像素
* @parm int pointY 用印Y坐标,单位为像素
* @parm int angle  转章角度(0-360)顺时针
* @return	成功: 0
*				失败: 非0
*/
typedef int (*pmanualPrintStart)(int printNum, int pointX, int pointY, int angle);

/** No.10
* 函数:	manualPrintStartByLight	
* 功能:	用印—光敏章用印
* 说明:	手工用印采用剪裁后 jpg 图像的盖章坐标点为印章的中心点基于图片左上角,以像素为单位
*			本接口为光敏章用印接口,用印时无需蘸印泥
*			手工用印采用剪裁后jpg图像的盖章坐标点为印章的中心点基于图片左上角,以像素为单位
*			转换后的盖章坐标点通过函数生成
* 参数:	
* @parm int printNum 卡槽号
* @parm int pointX 用印X坐标,单位为像素
* @parm int pointY 用印Y坐标,单位为像素
* @parm int angle  转章角度(0-360)顺时针
* @return	成功: 0
*				失败: 非0
*/
typedef int (*pmanualPrintStartByLight)(int printNum, int pointX, int pointY, int angle);

/** No.11
* 函数:	autoPrintStart	
* 功能:	用印—自动
* 说明:	自动用印的盖章坐标点为印章的中心点基于凭证左上角,以mm为单位.
*			用印机可支持自动并计算盖章角度
* 参数:	
* @parm int printNum 卡槽号
* @parm int pointX 用印X坐标,单位为 mm
* @parm int pointY 用印Y坐标,单位为 mm
* @parm int angle  旋转角度
* @return	成功: 0
*				失败: 非0
*/
typedef int (*pautoPrintStart)(int printNum, int pointX, int pointY, int angle);

/** No.12
* 函数:	openMachineBackDoor	
* 功能:	打开后门锁
* 说明:	打开后门锁以便对印控机中的印章,印泥等部件进行维护
*
* 参数:	
* @parm void
* @return	成功: 0
*				失败: 非0
*/
typedef int (*popenMachineBackDoor)(void);

/** No.13
* 函数:	openMachineBackDoorUnNormal	
* 功能:	打开后门锁(异常开锁)
* 说明:	异常开锁时记录异常开锁信息
*
* 参数:	
* @parm const char* openInfo 异常信息保存至用印机内,多条记录以";"分割,
*                           用印机内最多保留 4 条异常信息,每条异常信息 15 个字符
* @return	成功: 0
*				失败: 非0
*/
typedef int (*popenMachineBackDoorUnNormal)(const char* openInfo);

/** No.14
* 函数:	getMachineNum	
* 功能:	查询设备编号
* 说明:	获取设备编号, 返回值为 20 位以内的字符串
*
* 参数:	
* @parm void
* @return	成功: 成功返回编号,失败返回负值
*/
typedef char* (*pgetMachineNum)(void);

/** No.15
* 函数:	getMachineType	
* 功能:	获取设备型号
* 说明:	获取设备型号,此型号与用印机机身型号一致
*
* 参数:	
* @parm void
* @return	成功: 成功返回型号,失败返回"-@#"+错误码
*/
typedef char* (*pgetMachineType)(void);

/** No.16
* 函数:	checkPaperDoorState	
* 功能:	查询纸板是否关闭
* 说明:	用印前检测纸板是否关闭
*
* 参数:	
* @parm void
* @return	1---未关闭, 2---关闭,其它---错误
*/
typedef int (*pcheckPaperDoorState)(void);

/** No.17
* 函数:	checkBackDoorState	
* 功能:	查询后门锁 状态
* 说明:	获取后门锁的状态(处于关闭还是开启)
*
* 参数:	
* @parm void
* @return	1---未关闭, 2---关闭, 其它---错误
*/
typedef int (*pcheckBackDoorState)(void);

/** No.18
* 函数:	lockPrinter	
* 功能:	用印机锁定
* 说明:	用印机锁定时不允许其他程序使用用印机用印
*
* 参数:	
* @parm void
* @return	0---成功, -1---失败
*/
typedef int (*plockPrinter)(void);

/** No.19
* 函数:	unLockPrinter	
* 功能:	用印机解除锁定
* 说明:	用印机解除锁定时,其他进程可驱动用印机用印
*
* 参数:	
* @parm void
* @return	0---成功, -1---失败,其它错误码
*/
typedef int (*punLockPrinter)(void);

/** No.20
* 函数:	checkLockState	
* 功能:	查询用印机锁定状态
* 说明:	检查用印机锁定状态
*
* 参数:	
* @parm void
* @return	0---锁定, -1---未锁定, 其它错误码
*/
typedef int (*pcheckLockState)(void);

/** No.21
* 函数:	openVideoCapLight	
* 功能:	打开摄像头照明灯
* 说明:	当摄像头照明灯处于关闭状态,如果想开启摄像头照明灯可调用此接口
*
* 参数:	
* @parm void
* @return	0---成功,非0---失败,
*/
typedef int (*popenVideoCapLight)(void);

/** No.22
* 函数:	closeVideoCapLight	
* 功能:	关闭摄像头照明灯
* 说明:	当摄像头照明灯处于开启状态，如果想关闭摄像头照明灯可调用此接口
*
* 参数:	
* @parm void
* @return	0---成功,非0---失败,
*/
typedef int (*pcloseVideoCapLight)(void);

/** No.23
* 函数:	geterrMsg	
* 功能:	获取错误信息
* 说明:	获取错误码对应的错误信息
*
* 参数:	
* @parm int errNo 错误码
* @return	返回错误码对应的错误信息
*/
typedef char* (*pgeterrMsg)(int errNo);

/** No.24
* 函数:	connMachine	
* 功能:	打开设备连接
* 说明:	打开与设备的连接，设备使用时打开连接
*
* 参数:	
* @parm const char* seriaID 设备序列号
* @return	0---成功, -1---失败, 其它错误码
*/
typedef  int (*pconnMachine)(const char* seriaID);

/** No.25
* 函数:	disconnMachine	
* 功能:	关闭设备连接
* 说明:	断开与设备的连接,设备第一次使用或更换印章时调用
*
* 参数:	
* @parm void
* @return	0---成功, -1---失败, 其它错误码
*/
typedef int (*pdisconnMachine)(void);

/** No.26
* 函数:	isConnMachine	
* 功能:	设备连接检查
* 说明:	检查设备的连接状态
*
* 参数:	
* @parm void
* @return	0---成功, -1---失败, 其它错误码
*/
typedef int (*pisConnMachine)(void);

//===============CAMEAR API===============

/** No.27
* 函数:	openVideoCap	
* 功能:	打开摄像头
* 说明:	打开摄像头
*
* 参数:	
* @parm void
* @return	0---连接, 1---关闭, 其它错误码
*/
typedef int (*popenVideoCap)(void);

/** No.28
* 函数:	setVedioProperties	
* 功能:	设置摄像头属性– 自动调节功能 
* 说明:	每个摄像头属性都有其默认值,如果想修改摄像头的亮度、对比度
			色调、饱和度、清晰度、白平衡、曝光值等属性的值可调用此接口
			保存在本地的配置文件,每次 OCX 从配置文件中读取
*
* 参数:	
* @parm int brightness		亮度
* @parm int constrast		对比度
* @parm int hue				色调
* @parm int saturation		饱和度
* @parm int sharpness		清晰度
* @parm int whitebalance白平衡
* @parm int gain				曝光值
* @return	0---成功,非 0---失败, 
*/
typedef int (*psetVedioProperties)(int brightness, int constrast, int hue ,int saturation,int sharpness ,int whitebalance, int gain);

/** No.29
* 函数:	getImageFormat	
* 功能:	设置摄像头属性– 自动调节功能 
* 说明:	根据文件名、文件类型、是否处理等信息在指定全路径下生成用印前
* 用印后、图像处理、图像不处理等类型的图像文件
* 当在指定路径下生成图片文件后, 即可通过图片全路径获取图片
* 若无外部摄像头,则可不支持外部分摄像头参数.
* 补充说明：当 type=0 时,除生成一张 bmp 图片外,同时自动生成一张同名(后缀.jpg)剪裁 jpg 图片.
*
* 参数:	
* @parm const char*  filePath	文件名称(带目录的文件名,做为输入参数)
* @parm int type						文件类型0-BMP, 1-JPG ,2-Raw(原始文件)
* @parm int isEraseBorder		是否进行图像处理(纠偏,裁剪,去黑边)0---否 1---是
* @return	0---成功,非0---失败, 
*/
typedef int (*pgetImageFormat)(const char* filePath, int type, int isEraseBorder);

/** No.30
* 函数:	revolveImg	
* 功能:	旋转图像
* 说明:	将原图像按顺时针旋转指定角度
*
* 参数:	
* @parm const char*  filePath		源文件路径
* @parm const char*  targetPath	目标文件路径
* @parm int angle						旋转角度(0-360)
* @return	0---成功,非0---失败, 
*/
typedef int (*prevolveImg)(const char* filePath, const char*  targetPath, int angle);

/** No.31
* 函数:	closeVideoCap	
* 功能:	关闭摄像头
* 说明:	关闭摄像头
*
* 参数:	
* @parm void
* @return	0--成功, 非 0--失败
*/
typedef int (*pcloseVideoCap)(void);

/** No.32
* 函数:	checkVideoState	
* 功能:	查询摄像头状态
* 说明:	获取摄像头的状态(处于关闭还是开启)
*
* 参数:	
* @parm void
* @return	1---打开, 2---关闭, 其它---错误
*/
typedef int (*pcheckVideoState)(void);

//===============ABNORMAL INFOMATION===============
/** No.33
* 函数:	readOpenBackDoorExceptionInfo	
* 功能:	读取异常开锁记录
* 说明:	每行信息为一条开锁记录
*
* 参数:	
* @parm void
* @return	1---打开, 2---关闭, 其它---错误
*/
typedef char* (*preadOpenBackDoorExceptionInfo)(void);

/** No.34
* 函数:	readOpenBackDoorExceptionInfo	
* 功能:	删除所有异常开锁信息
* 说明:	删除所有异常开锁信息
*
* 参数:	
* @parm void
* @return	0-成功 -1-失败 2-其他
*/
typedef int (*pdelOpenBackDoorExceptionInfo)(void);





class  PsbcStamp
{
public:
	PsbcStamp();
	~PsbcStamp();
public:
	

	int initializationMachine(void);
	char * querySealInfo(const char* machineNum);
	char* querySlotInfo(const char* machineNum);
	int initStamp(const char* machineNum,const char* slotNumAndStampId);
	int setAcrossPageSeal(void);
	int setMachineNum(const char* machineNum);
	int openPaperDoor(void);
	int checkManualPrintPara( int pointX, int pointY, int angle);
	int manualPrintStart(int printNum, int pointX, int pointY, int angle);
	int manualPrintStartByLight(int printNum, int pointX, int pointY, int angle);
	int autoPrintStart(int printNum, int pointX, int pointY, int angle);
	int openMachineBackDoor(void);
	int openMachineBackDoorUnNormal(const char* openInfo);
	char* getMachineNum(void);
	char* getMachineType(void);
	int checkPaperDoorState(void);
	int checkBackDoorState(void);
	int lockPrinter(void);
	int unLockPrinter(void);
	int checkLockState(void);
	int openVideoCapLight(void);
	int closeVideoCapLight(void);
	char* geterrMsg(int errNo);
	int connMachine(const char* seriaID);
	int disconnMachine(void);
	int isConnMachine(void);
	int openVideoCap(void);
	int setVedioProperties(int brightness, int constrast, int hue ,int saturation,int sharpness ,int whitebalance, int gain);
	int getImageFormat(const char* filePath, int type, int isEraseBorder);
	int revolveImg(const char* filePath, const char*  targetPath, int angle);
	int closeVideoCap(void);
	int checkVideoState(void);
	char* readOpenBackDoorExceptionInfo(void);
	int delOpenBackDoorExceptionInfo(void);
private:
	void GetCurrentOcxDir( char * path);
	void LoadLibrarypsbc();
private:
	pinitializationMachine  _pinitializationMachine;
	pquerySealInfo _pquerySealInfo;
	pquerySlotInfo _pquerySlotInfo;
	pinitStamp _pinitStamp;
	psetAcrossPageSeal _psetAcrossPageSeal;
	psetMachineNum _psetMachineNum;
	popenPaperDoor _popenPaperDoor;
	pcheckManualPrintPara _pcheckManualPrintPara;
	pmanualPrintStart _pmanualPrintStart;
	pmanualPrintStartByLight _pmanualPrintStartByLight;
	pautoPrintStart _pautoPrintStart;
	popenMachineBackDoor  _popenMachineBackDoor;
	popenMachineBackDoorUnNormal _popenMachineBackDoorUnNormal;
	pgetMachineNum _pgetMachineNum;
	pgetMachineType _pgetMachineType;
	pcheckPaperDoorState  _pcheckPaperDoorState;
	pcheckBackDoorState  _pcheckBackDoorState;
	plockPrinter  _plockPrinter;
	punLockPrinter  _punLockPrinter;
	pcheckLockState  _pcheckLockState;
	popenVideoCapLight _popenVideoCapLight;
	pcloseVideoCapLight _pcloseVideoCapLight;
	pgeterrMsg _pgeterrMsg;
	pconnMachine _pconnMachine;
	pdisconnMachine _pdisconnMachine;
	pisConnMachine  _pisConnMachine;
	popenVideoCap _popenVideoCap;
	psetVedioProperties _psetVedioProperties;
	pgetImageFormat _pgetImageFormat;
	prevolveImg _prevolveImg;
	pcloseVideoCap  _pcloseVideoCap;
	pcheckVideoState _pcheckVideoState;
	preadOpenBackDoorExceptionInfo _preadOpenBackDoorExceptionInfo;
	pdelOpenBackDoorExceptionInfo _pdelOpenBackDoorExceptionInfo;
	HINSTANCE _dllIns;
};

class PromptInfo
{
public:
	PromptInfo()
	{
		_mapPromptInfo.insert(make_pair(0,"成功"));
		_mapPromptInfo.insert(make_pair(1,"进纸门开"));
		_mapPromptInfo.insert(make_pair(2,"进纸门关"));
		_mapPromptInfo.insert(make_pair(3,"安全门开"));
		_mapPromptInfo.insert(make_pair(4,"安全门关"));
		_mapPromptInfo.insert(make_pair(5,"用印机锁定"));
		_mapPromptInfo.insert(make_pair(6,"用印机未锁定"));
		_mapPromptInfo.insert(make_pair(7,"设备连接成功"));
		_mapPromptInfo.insert(make_pair(8,"设备未连接"));
		_mapPromptInfo.insert(make_pair(9,"打开摄像头成功"));
		_mapPromptInfo.insert(make_pair(10,"打开摄像头失败"));
		_mapPromptInfo.insert(make_pair(11,"关闭摄像头成功"));
		_mapPromptInfo.insert(make_pair(12,"关闭摄像头失败"));
		_mapPromptInfo.insert(make_pair(-1,"失败"));
		_mapPromptInfo.insert(make_pair(-2,"参数不合法"));

	}

	map<int,string> _mapPromptInfo;
};
#endif