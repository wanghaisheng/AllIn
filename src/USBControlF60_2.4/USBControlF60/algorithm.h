
/************************************************* 
Copyright:yuanyi 
Author: 
Date:2016-05-24 
Description: 根数据计算相关的功能 
**************************************************/  

#ifndef  _ALGORITHE_H
#define  _ALGORITHE_H
#include <string>
using namespace std; 
typedef unsigned int Uint;
typedef unsigned short UInt16;
typedef unsigned char byte;
typedef unsigned long UInt32;//

struct svector
{
	double _x;
	double _y;
};

namespace algorithm
{
	namespace  convert
	{
		//0-F转换为0-16胡数字
		int Char16ToInt(char c)  ;
		//16进制字符串转换为整数输出
		int StrToNumber16(const char *str);
		//0-F转换为数字
		char ConvertHexChar(char ch);
		//字符串转换为16进制
		int  str2hex(char *str1, unsigned char *p);
		//16进制转换为字符输出
		void hex2str(char *ptr, unsigned char *buf, int len);
		//转换为小端模式的字符串
		UInt16 LeBufToU16( byte Buf[] , int offset);
		//转换32bit数据为无符号long
		UInt32 LeBufToU32(byte Buf[] , int offset);
		//unsigned long转换为32bit数据
		void U32ToLeBuf(UInt32 val ,byte Buf[] , int offset);
		//unsigned short转换为16bit数据
		void U16ToLeBuf(UInt16 val, byte Buf[] , int offset);
		//四舍五入
		double round(double val);
	}

	namespace math
	{
		//计算两点之间的距离
		double  TwoPointsDistance(double firstX,double firstY , double secondX ,double secondY);
		//计算两点之间的夹角
		double  CalculateVectorAngle(svector a,svector b);
		//计算位置
		double  CalculatePosition(svector a ,svector b,double length ,double angle ,double angle2,svector& sPos);
		//旋转
		double  rotate(double rPointx,double rPointy,double orgX,double orgY,double raAngle,double & outX,double & outY);
	}

	namespace project
	{
        int  ZeroArrayJudge(char *arry ,int len);
	}

	//  =(x1x2+y1y2)/[√(x1?+y1?)*√(x2?+y2?)] 
}
#endif