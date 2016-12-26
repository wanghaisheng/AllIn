// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 RZCAMERA_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// RZCAMERA_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#ifndef _RZCAMERA_H
#define _RZCAMERA_H

#ifdef RZCAMERA_EXPORTS
#define RZCAMERA_API extern "C" __declspec(dllexport) int 
#else
#define RZCAMERA_API extern "C" __declspec(dllimport) int 
#endif

enum     CAMPARAM {  
	brightness				= 0,
	contrast				= 1,
	hue						= 2,
	saturation				= 3,
	sharpness				= 4,
	whitebalance			= 6,
	backlightcompensation	= 7,
	zoom					= 10,
	offsetY					= 13,
	offsetX					= 14,
	expouse					= 15,
	gamma					= 16,
	colormode				= 17,
	flipmode				= 18,
	aeing					= 19,
	awbing					= 20
};

enum CAMERAINDEX
{
	PAPERCAMERA,	//凭证摄像头
	ENVCAMERA,		//环境摄像头
	SIDECAMERA		//侧门摄像头
};

enum IMAGEFORMAT
{
	FORMATJPG,		//JPG格式
	FORMATBMP		//BMP格式
	};

#define RET_SUCCESS 0

/*==============================================================
函数:OpenCamera	
功能:打开摄像头

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API OpenCamera(IN CAMERAINDEX camType);

/*==============================================================
函数:CloseCamera	
功能:关闭摄像头

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API CloseCamera(IN CAMERAINDEX camType);

/*==============================================================
函数:StartPreview	
功能:开始预览

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）
		nWidth		窗口宽度
		nHeight		窗口高度
		hwndParent	父窗口句柄

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API StartPreview(IN CAMERAINDEX camType,IN int nWidth,IN int nHeight, IN HWND hwndParent);

/*==============================================================
函数:StopPreview	
功能:开始预览

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API StopPreview(IN CAMERAINDEX camType);

/*==============================================================
函数:SetResolution
功能:设置分辨率

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）
		nWidth		宽度
		nHeight		高度

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API SetResolution(IN CAMERAINDEX camType,ULONG nWidth, ULONG nHeight);

/*==============================================================
函数:SetDPIValue	
功能:设置JPG图片的DPI值

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）
		nDPIx       为宽的DPI值
		nDPIy       为高的DPI值

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API SetDPIValue(IN CAMERAINDEX camType, IN int nDPIx, IN int nDPIy);

/*==============================================================
函数:SetParameter	
功能:设置相机参数

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）
		DLParam     参数类型
		lvalue      参数的值

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API SetParameter(IN CAMERAINDEX camType, CAMPARAM  CamParam, long lvalue);

/*==============================================================
函数:CapturePhoto	
功能:拍照

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）
		imgFormat	保存图片格式(0：JPG格式，1：BMP格式)
		szPath		包含扩展名的全路径

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API CapturePhoto(IN CAMERAINDEX camType, IN IMAGEFORMAT imgFormat, IN char *szPath);

/*==============================================================
函数:StartCaptureVideo	
功能:开始录像

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）
		szPath		包含扩展名的全路径(.avi格式)

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API StartCaptureVideo(IN CAMERAINDEX camType, IN char *szPath);

/*==============================================================
函数:StopCaptureVideo	
功能:停止录像

参数:	camType		摄像头类型（0：凭证摄像头，1：环境摄像头，2：侧门摄像头）

返回值     0:成功  
	    其它:失败		
--------------------------------------------------------------*/
RZCAMERA_API StopCaptureVideo(IN CAMERAINDEX camType, IN char *szPath);

RZCAMERA_API RegisterPlugin();


#endif