#pragma once

#include <string.h>

//调试信息头
#define CMD_DEBUG_HEADER ((unsigned char)0x9F)		//调试协议头 

// 主机发送命令码定义
#define CMD_HEADER									((unsigned char)0x7f)		//协议头
#define CMD_END											((unsigned char)0x7e)		//协议尾

//主机接收命令码定义
#define CMD_RECVHEADER									((unsigned char)0x8f)		//协议头
#define CMD_RECVEND											((unsigned char)0x8e)		//协议尾

//错误  当协议头出错，将cmd置为该值
#define USB_RECEIVE_ERR							((unsigned char)0xFF)		

//命令码
enum CommandType{
	CMD_QUIT_MAINTAIN = 1,		//退出维护模式				
	CMD_RESTART,				//重启印章机
	CMD_GETSYSTEMMSG,			//设备系统信息
	CMD_GETSYSVER,				//固件版本号
	CMD_SETDEVSN,				//设置设备序列号
	CMD_GETDEVSN,				//获取设备序列号
	CMD_QUERY_DEVSTATUS,		//设备状态
	CMD_SET_DEVPROTECT,			//进入维护模式
	CMD_GET_INFRARED_STATUS,	//获取红外状态
	CMD_SEAL_PRESENT,			//获取印章状态
	CMD_DEV_DOOR_STATUS,		//获取纸门和安全门状态
	CMD_OPENDOOR_PAPER,			//推出纸门
	CMD_OPENDOOR_SAFE,			//打开安全门
	CMD_CLOSENDOOR_SAFE,		//关闭安全门
	CMD_LIGHT_CTRL,				//补光灯开关
	CMD_X_MOVE,					//X方向移动
	CMD_Y_MOVE,					//y方向移动
	CMD_STAMPER_TURN,			//转章
	CMD_BEGINSTAMP_NORMAL,		//普通盖章
	CMD_SELECTSTAMP,			//选章
	CMD_BEGINSTAMP_CANCLE,		//终止盖章
	CMD_BEEP_CTRL,				//蜂鸣器
	CMD_LIGHTING_BRIGHTNESS,	//补光灯亮度调节
	CMD_SEALBACK,				//印章归位
	CMD_INTESTMODE,				//进入测试模式
	CMD_OUTTESTMODE,			//退出测试模式
	CMD_GETMAC,					//获取设备存储的Mac地址
	CMD_MACPCBIND,				//绑定MAC地址
    CMD_ALARMPD = 0x1D,			//设置报警功能
	CMD_SETDEVCODE = 0x1E,		//设置机器认证码
	CMD_GETDEVCODE = 0x1F,	    //获取机器认证码
	CMD_STORE_DATA = 0x20,		//保存数印章据到印章机存储其中
	CMD_READ_DATA = 0x21,		//读取存储区数据
	CMD_GETSTORECPACITY=0x22,	//获取存储区容量,以及版本号
	CMD_ENTERAUTH=0x23,		    //进入认证状态
	CMD_QUITAUTH=0x24,			//退出认证状态
    CMD_STAMPERNO=0x25,         //印章仓位号
    CMD_SPECIFYSTAMPERNO=0x26,  //指定印章仓位号
    CMD_OPERATEBLOCK=0x27,      //操作指定块的绝对
    CMD_VERIFYKEY = 0x28,       //验证密钥
    CMD_READBLOCK = 0x29,       //读指定块
    CMD_WRITEBLOCK= 0x2A,       //写指定块
	CMD_STAMPRANGE=	0x2B,		//读取盖章范围									
	CMD_SETMAPPING= 0x2C,		//设置章映射关系表
	CMD_READMAPPINT= 0x2D,		//获取映射关系
	CMD_WRITEMEMORY= 0x2E,      //写内存区
	CMD_READMEMORY= 0x2F,		//读内存区	
    CMD_ALL_RFID = 0x30,        //读取当前所有RFID, 包括侧门
    CMD_ALARM_VOLTAGE = 0x31,   //读取报警器电池电压值
    CMD_CLEAR_STATISTIC = 0x32, //盖章统计信息清零
    CMD_RESET = 0x33,           //复位
    CMD_CONFIRM_STAMP = 0x34,   //盖章信息确认
    CMD_READ_STAMPING       = 0x35,     //读盖章信息
    CMD_SET_SIDEDOOR_INFO   = 0x36,     //设置侧门开门提示信息
    CMD_PAPER_DOOR_INFO     = 0x37,     //设置推纸门开启动超时提示
    CMD_ALARM_STATUS        = 0x38,     //读取报警器控制状态
    CMD_HARDWARE_VERSION    = 0x39,     //硬件版本号

	CMD_SET_FACTORY_TEST = 0x51,//设置工厂测试模式,不需要电脑即可循环盖章测试P1=1使能P1=0 禁用
    CMD_SET_USB_DEBUG = 0x52,   //底层调试信息开关

    CMD_READ_BACKUP_SN = 0xD0,  //读备用板序列号
    CMD_WRITE_BACKUP_SN = 0xD1, //写备用板序列号
};


//升级命令码
enum CommandFirewaeUpdate{
	CMD_UPDATE_NOTICE = 0xB1,	//通知升级
	CMD_QUERYUPDATE_FLAG,		//查询是否可以升级	
	CMD_FIREWARE_SENDDATA,		//升级下发数据
	CMD_FIREWARE_COMPLETE,		//确认是否升级完成
	CMD_FIREWARE_MCUCHECK		//MCU校验
};

//从机异步返回
enum CommandResultAsyn {
	CMD_STAMPER_DOWN = 0xA0,	        //盖章已下压	
	CMD_STAMPER_ARMBACK,		        //机械手臂归位
	CMD_STAMPER_COMPLETE = 0xA2,		//盖章完成
	CMD_STAMPER_ERR = 0xA3,			    //盖章过程中掉章
	CMD_STAMPER_PAPERDOORCLOSE = 0xA4,  //纸门关闭
	CMD_STAMPERPROC_ERROR = 0xA5,	    //盖章错误
    CMD_SIDEDOOR_CLOSE = 0xA6,          //安全门关闭
    CMD_TOP_CLOSE = 0xA7,               //顶盖门关闭
    CMD_ELEC_LOCK = 0xA8,               //电子锁上锁通知
};


//最大参数长度
#define PARAM_DATA_LEN 59
//数据发送包
typedef struct _tagSendPackage
{
	unsigned char m_head;
	unsigned char m_cmd;
	unsigned char m_len;
	unsigned char m_data[PARAM_DATA_LEN];
	unsigned char m_checksum;
	unsigned char m_end;
public:
	_tagSendPackage(){
		m_head = 0;
		m_cmd = 0;
		m_len = 0;
		memset(m_data, 0, sizeof(m_data));
		m_checksum = 0;
		m_end = 0;
	};
	_tagSendPackage(unsigned char cmd, unsigned char len = 0, unsigned char* pData = 0){
		m_head = CMD_HEADER;
		m_cmd = cmd;
		m_len = PARAM_DATA_LEN >  len ? len : PARAM_DATA_LEN;
		memset(m_data, 0, sizeof(m_data));
		memcpy(m_data, pData, len);

		m_checksum = m_head + m_cmd + m_len;

		//m_checksum = 0;
		for (int i = 0; i < PARAM_DATA_LEN/*m_len*/; ++i)
		{
			m_checksum += m_data[i];
		}
		m_end = CMD_END;
	};

	//赋值操作符重载
	_tagSendPackage &operator =(const _tagSendPackage &_SendPackage)
	{
		if (this != &_SendPackage)
		{
			this->m_head	= _SendPackage.m_head;
			this->m_cmd		= _SendPackage.m_cmd;
			this->m_len		= _SendPackage.m_len;
			memset(this->m_data, 0, sizeof(this->m_data));
			memcpy(this->m_data, _SendPackage.m_data, _SendPackage.m_len);
			this->m_checksum	= _SendPackage.m_checksum;
			this->m_end				= _SendPackage.m_end;
		}

		return *this;
	}

}SendPackage, *LPSendPackage;

//数据接受包
typedef struct _tagReceivePackage
{
	unsigned char m_cmd;
	unsigned char m_len;
	unsigned char m_data[PARAM_DATA_LEN];
	unsigned char m_m_checksum;
	unsigned char m_end;

public:
	_tagReceivePackage(unsigned char cmd = 0, unsigned char len = 0, unsigned char* pData = 0){	
		m_cmd = cmd;
		m_len = PARAM_DATA_LEN >  len ? len : PARAM_DATA_LEN;
		memset(m_data, 0, sizeof(m_data));
		if(pData) memcpy(m_data, pData, len);
		m_m_checksum = 0;
		m_end = 0;
	};

	//赋值操作符重载
	_tagReceivePackage &operator =(const _tagReceivePackage &_RecPackage)
	{
		if (this != &_RecPackage)
		{
			this->m_cmd	= _RecPackage.m_cmd;
			this->m_len		= _RecPackage.m_len;
			memcpy(this->m_data, _RecPackage.m_data,_RecPackage.m_len);
			this->m_m_checksum = _RecPackage.m_m_checksum;
			this->m_end = _RecPackage.m_end;
		}

		return *this;
	}

}ReceivePackage, *LPReceivePackage;
