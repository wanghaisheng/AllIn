#include "stdafx.h"
#include "algorithm.h"
#include <math.h>
#define PI  3.1415926

namespace algorithm
{
	namespace convert
	{

		char  ConvertHexChar(char ch) 
		{
			if((ch>='0')&&(ch<='9'))
				return ch-0x30;
			else if((ch>='A')&&(ch<='F'))
				return ch-'A'+10;
			else if((ch>='a')&&(ch<='f'))
				return ch-'a'+10;
			else
				return (16);
		}

		int   str2hex(char *str1, unsigned char *p)
		{
			std::string str(str1);
			int hexdata,lowhexdata;
			int hexdatalen=0;
			int len=str.length();
			//Senddata.SetSize(len/2);
			for(int i=0;i<len;)
			{
				char lstr,hstr=str[i];
				if(hstr==' ')
				{
					i++;
					continue;
				}
				i++;
				if(i>=len)
					break;
				lstr = str[i];
				hexdata = ConvertHexChar(hstr);
				lowhexdata = ConvertHexChar(lstr);
				if((hexdata == 16) || (lowhexdata == 16))
					break;
				else 
					hexdata = hexdata*16 + lowhexdata;
				i++;
				p[hexdatalen]=(char)hexdata;
				hexdatalen++;
			}
			//Senddata.SetSize(hexdatalen);
			return hexdatalen;
		}

		void   hex2str(char *ptr, unsigned char *buf, int len)
		{
			for(int i = 0; i < len; i++)
			{
				sprintf(ptr, "%02x",buf[i]);
				ptr += 2;
			}
		}

		int Char16ToInt(char c)  
		{  
			int num;  
			num = 0;//  
			switch (c)  
			{  
			case '0':  
				num = 0;  
				break;  
			case '1':  
				num = 1;  
				break;  
			case '2':  
				num = 2;  
				break;  
			case '3':  
				num = 3;  
				break;  
			case '4':  
				num = 4;  
				break;  
			case '5':  
				num = 5;  
				break;  
			case '6':  
				num = 6;  
				break;  
			case '7':  
				num = 7;  
				break;  
			case '8':  
				num = 8;  
				break;  
			case '9':  
				num = 9;  
				break;  
			case 'a':  
			case 'A':  
				num = 10;  
				break;  
			case 'b':  
			case 'B':  
				num = 11;  
				break;  
			case 'c':  
			case 'C':  
				num = 12;  
				break;  
			case 'd':  
			case 'D':  
				num = 13;  
				break;  
			case 'e':  
			case 'E':  
				num = 14;  
				break;  
			case 'f':  
			case 'F':  
				num = 15;  
				break;  
			default:  
				break;  
			}  
			return num;  
		}  

		int StrToNumber16(const char *str)  
		{  
			int len,i,num;  
			num = 0;//使用数据必须初始化否则产生不确定值  
			len = strlen(str);  
			for (i = 0; i < len; i++)  
			{  
				num = num*16 + Char16ToInt(str[i]);/*十六进制字符串与10进制的对应数据*/   
			}  
			return num;  

		}  

		UInt16 LeBufToU16(byte Buf[], int offset)
		{
			UInt16 val;
			byte Ch;
			Ch = Buf[offset + 1];
			val = Ch;
			val <<= 8;
			val |= Buf[offset];
			return val;
		}
		UInt32 LeBufToU32(byte Buf[], int offset)
		{
			UInt32 val;
			byte Ch;
			Ch = Buf[offset + 3];
			val = Ch;
			val <<= 8;

			val |= Buf[offset + 2];
			val <<= 8;

			val |= Buf[offset + 1];
			val <<= 8;

			val |= Buf[offset + 0];            
			return val;
		}
		void U32ToLeBuf(UInt32 val ,byte Buf[], int offset)
		{            
			byte Ch;
			Ch = (byte)(val >> 24);
			Buf[offset + 3] = Ch;
			Ch = (byte)(val >> 16);
			Buf[offset + 2] = Ch;
			Ch = (byte)(val >> 8);
			Buf[offset + 1] = Ch;
			Ch = (byte)(val >> 0);
			Buf[offset + 0] = Ch;            
		}
		void U16ToLeBuf(UInt16 val, byte Buf[], int offset)
		{
			byte Ch;            
			Ch = (byte)(val >> 8);
			Buf[offset + 1] = Ch;
			Ch = (byte)(val >> 0);
			Buf[offset + 0] = Ch;
		}
		double round(double val)
		{ 
			return (val> 0.0) ? floor(val+ 0.5) : ceil(val- 0.5);
		}


	}

	namespace math
	{
		double  TwoPointsDistance(double firstX,double firstY , double secondX ,double secondY)
		{
			return sqrt((secondX-firstX)*(secondX-firstX)+(secondY-firstY)*(secondY-firstY));
		}
		//返回值为弧度值
		double  CalculateVectorAngle(svector a,svector b)
		{
			double  tt,kk,zz;
			double ssum;
			tt=a._x*a._x+a._y*b._y;
			kk=b._x*b._x+b._y*b._y;
			zz=sqrt(tt)*sqrt(kk);
			tt=a._x*b._x+a._y*b._y;
			ssum=tt/kk;
			tt=ssum;
			return  tt;
		}

		double  CalculatePosition( svector a,svector b,double length ,double angle1 ,double angle2,svector& sPos)
		{

			double length1 = length;
			double lenght2 = TwoPointsDistance(0,0,a._x,a._y);
			double length3 = TwoPointsDistance(0,0,b._x,b._y);
			double tempstep1= angle1*(length1+lenght2);
			double tempstep2= angle2*(length1+length3);
			double aP = 0 ;
			double bP = 0 ;
			double cP = 0 ;
			double dP = 0 ;
			double dcTemp1;
			double dcTemp2;	

			aP = angle1*lenght2*length ; 
			bP = angle2*length3*length ;
			cP = aP -bP*a._x/b._x;
			dP=  a._y - a._x*b._y/b._x;
			dcTemp1 = cP/dP;
			sPos._y = dcTemp1;
			sPos._x = sqrt(length*length - sPos._y* sPos._y);



			return  0;	
		}


		//假设对图片上任意点(x,y)，绕一个坐标点(rx0,ry0)逆时针旋转a角度后的新的坐标设为(x0, y0)，有公式
		double  rotate(double rPointx,double rPointy,double orgX,double orgY,double raAngle,double & outX,double & outY)
		{
			 outX= (rPointx - orgX)*cos(raAngle) - (rPointy - orgY)*sin(raAngle) + orgX ;

             outY= (rPointx - orgX)*sin(raAngle) + (rPointy - orgY)*cos(raAngle) + orgY ;

			 return 0;
		}

	}


	namespace project
	{
		int  ZeroArrayJudge(char * arry ,int len)
		{

			for(unsigned int i = 0 ; i<len; ++i)
			{
				if(arry[i] != 0)
				{

					return 1 ;
				}
			}

			return 0;
		}

	}
}