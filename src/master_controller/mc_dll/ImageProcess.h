#ifndef ImageProcess_H_
#define ImageProcess_H_
#ifdef IMAGEPROCESS_EXPORTS
#define MYLIBDLL extern "C" _declspec(dllexport) 
#else
#define MYLIBDLL extern "C" _declspec(dllimport) 
#endif
#include <atltypes.h>
#include <gdiplus.h>

#define __ImageProcessDllVersion301__	1
#define JUDGE_PAPER_A4	2000
typedef int (CALLBACK *MotionDetectRev)(unsigned int uMsg,char* lParam);//移动侦测消息回调

//函数及接口说明
/************************************************************************
 * 描述:票据自动旋转并切割黑边（对A4进行旋转处理）
 * 输入:
		*srcName 源图像全路径名称 如：d:\1.jpg
        *dstName 切出的票据路径
 * 输出
        返回 0 成功
		返回 -1 图片加载失败
 ************************************************************************/
MYLIBDLL int STRoateAndExtatBillImage(const char* srcName, const char* dstName);
/**********************************************************************
 * 检测票据是否符合某个版式,读取xml文件配置
 * 输入:
		*src_file_name:输入待确认文件名
		*ResultData:输出匹配信息(模板名_模板ID_匹配率)；eg.测试票据_1_0.801
		*_Angel:输出票据角度(正:0,反:180)
		*stamppos:输出用印位置(如x:123,y:456,则输出:123@456)
 * 输出:
		* 0-OK
		*-1-ERROR,源图像路径不存在
		*-2-ERROR,传入了不支持的模板类型
		*-3-ERROR,读取模板错误
		*-8-ERROR,xml文件读取失败
		*-4-ERROR,所有模板识别率低于0.1
 *
 **********************************************************************/
MYLIBDLL int STAutoIdentifyEx2(const char* src_file_name,char *ResultData,char *_Angel,char* stamppos);

/**********************************************************************
 * 对票据指定区域进行识别
 * 输入:
		*src_file_name:输入票据文件名
		*PointString:识别区域左上方点+宽高（eg.100,100,200,200@）
		*OCRresult:区域识别结果

 * 输出:
		* 0-OK
		*-1-ERROR,源图像路径不存在
		*-2-ERROR,传入了不支持的模板类型
		*-3-ERROR,读取模板错误
 *
 **********************************************************************/
MYLIBDLL int  STImageAreaOCREx(char* src_file_name,char* PointString,char* OCRresult);
/**********************************************************************
 * 任意角度寻找盖章点
 * 输入:
		*Imagefile 原图片全路径 
		*chpos 相对票据中心点在图片中的相对位置；PS：不能等于0(如x:-123,y:456,则输入:-123@456)
		*chinputangle 图片切边后票据识别后，图片方向（0 正向，180 逆向）
 * 输出:
		*chstamppos (如x:-123,y:456,则输出:-123@456)
		*chStamAngel 返回章面顺时针旋转角度
		* 返回值：比较结果 ret;
		*		  -1，输入图片路径为空 
		*		  -2，图片读取失败

 *
 **********************************************************************/
//MYLIBDLL int STSeachSealPoint(char* Imagefile,int StamX,int StamY,int* StamPointX,int* StamPointY,double *StamAngel,int inputangle);
MYLIBDLL int STSeachSealPoint(char* Imagefile,char* chpos,char* chinputangle,char* chstamppos,char *chStamAngel);
/************************************************************************
// 函数名称：STMerge_image()
// 函数功能：两张图合并(建议保存为JPG格式)
// 参数说明：char *src_file_name1, 图片1路径
//           char *src_file_name2, 图片2路径
//			 dst_file_name,合并后图片路径
//			 
// 返 回 值：0
// 备    注：
************************************************************************/
MYLIBDLL int STMerge_image(const char* src_file_name1,const char* src_file_name2,const char* dst_file_name);

/************************************************************************
// 函数名称：RotateImageAngle()
// 函数功能：自定义角度旋转图像
// 参数说明：IplImage *src, 待旋转图像
//           double angle, 旋转角度（-360~360）
//           
// 返 回 值：IplImage *dst，旋转后图像
// 备    注：
************************************************************************/
MYLIBDLL int STRotateImageAngle(const char* srcName, const char* dstName, double angle);

/************************************************************************
 * 描述:票据手动切边
 * 输入:
		*srcName 源图像全路径名称 如：d:\1.jpg
		*PointString:识别区域左上方点+宽高（eg.100,100,200,200@）
        *dstName 切出的票据路径
 * 输出
        返回 0 成功
		返回 -1 图片加载失败
 ************************************************************************/
MYLIBDLL int STManualExtatBillImage(const char* srcName,const char* PointString, const char* dstName);

/************************************************************************
 * 转换图像DPI
 * 输入:
        *file_name:源图像绝对路径
		*quality:需要将图像尺寸转换的倍率
		*dpi:目标DPI的值
		*dst_file_name:目标图像绝对路径
		*int TypeFlag：源图片格式（默认为3，JPG）
 * 输出:
 * 0-OK
 *-1-ERROR 加载源图像失败
 *-2-ERROR 写目标图像失败
 *
 备注： 支持图片格式：
 //		CXIMAGE_FORMAT_BMP = 1,
 // 	CXIMAGE_FORMAT_GIF = 2,
 // 	CXIMAGE_FORMAT_JPG = 3,
 // 	CXIMAGE_FORMAT_PNG = 4,
 // 	CXIMAGE_FORMAT_ICO = 5,
 // 	CXIMAGE_FORMAT_TIF = 6,
 ************************************************************************/
MYLIBDLL int STSetImageDPI(const char* src_file_name,BYTE quality,int dpi,const char* dst_file_name,int TypeFlag=3);

/************************************************************************
// 函数名称：RotateImageAngle()
// 函数功能：自定义角度旋转图像
// 参数说明：IplImage *src, 待旋转图像
//           double angle, 旋转角度（-360~360）
//           int opt,选择旋转方式
// 返 回 值：IplImage *dst，旋转后图像
// 备    注：
************************************************************************/
MYLIBDLL int RotateImageAngle(const char* srcName, const char* dstName, double angle);

/************************************************************************
// 函数名称：ImageResizeScale()
// 函数功能：图像按比例变换
// 参数说明：char *src_file_name, 待处理图像
//           double scale, 比例(scale>0.0)
// 返 回 值：0
// 备    注：
************************************************************************/
MYLIBDLL int ImageResizeScale(const char* src_file_name,double scale);

/************************************************************************
// 函数名称：ImageResize()
// 函数功能：图像按指定宽高变换
// 参数说明：char *src_file_name, 待处理图像
//           int Newwidth,int Newheight, 输出图像宽高
// 返 回 值：0
// 备    注：(Newwidth>0)&&(Newheight>0)
************************************************************************/
MYLIBDLL int ImageResize(const char* src_file_name,int Newwidth,int Newheight);

/************************************************************************
// 函数名称：Blending_image()
// 函数功能：两张图片融合(建议保存为JPG格式)
// 参数说明：char *src_file_name, 被处理的图片
//           char *src_file_name1, LOG图片
//			 dst_file_name,融合后图像
//			 double alpha,src_file_name图片的深度,范围在（0.0~1.0）,src_file_name1图片的深度等于（1-alpha）
//			 CPoint beginPoint,LOG在被处理的图片融合起始位置 
// 返 回 值：0
// 备    注：
************************************************************************/
MYLIBDLL int Blending_image(const char* src_file_name,const char* src_file_name1,const char* dst_file_name,double alpha,CPoint beginPoint);


/************************************************************************
// 函数名称：Colortoblack_white()
// 函数功能：彩色图片转为黑白
// 参数说明：char *src_file_name, 待处理图像
//           char *src_file_name, 输出图像
// 返 回 值：0
// 备    注：
************************************************************************/
MYLIBDLL int Colortoblack_white(const char* src_file_name,const char* dst_file_name );

/************************************************************************
 * 转换图像DPI
 * 输入:
        *file_name:源图像绝对路径
		*rate:需要将图像尺寸转换的倍率
		*dpi:目标DPI的值
		*dst_file_name:目标图像绝对路径
		*int TypeFlag：源图片格式（默认为3，JPG）
 * 输出:
 * 0-OK
 *-1-ERROR 加载源图像失败
 *-2-ERROR 写目标图像失败
 *
 备注： 支持图片格式：
 //		CXIMAGE_FORMAT_BMP = 1,
 // 	CXIMAGE_FORMAT_GIF = 2,
 // 	CXIMAGE_FORMAT_JPG = 3,
 // 	CXIMAGE_FORMAT_PNG = 4,
 // 	CXIMAGE_FORMAT_ICO = 5,
 // 	CXIMAGE_FORMAT_TIF = 6,
 ************************************************************************/
MYLIBDLL int SetImageDPI(const char* src_file_name,double rate,int dpi,const char* dst_file_name,int TypeFlag=3);

/************************************************************************
 * 描述:检测票据四个顶点，并计算其透射矩阵，切出有用票据（自动切边）(线程切边，异步)
 * 输入:
		*srcName 源图像全路径名称 如：d:\1.jpg
        *dstName 切出的票据路径
        *
 * 输出
        返回 0 成功
		返回 -1 图片加载失败
 ************************************************************************/

MYLIBDLL int RoateAndExtatBillImage(const char* srcName, const char* dstName);


/************************************************************************
 * 描述:检测票据四个顶点，并计算其透射矩阵，切出有用票据（自动切边）
 * 输入:
		*srcName 源图像全路径名称 如：d:\1.jpg
        *dstName 切出的票据路径
        *FlagWebData 判断是否为HTTP图片，true为HTTP图片，false为本地图片，默认为false
 * 输出
        返回 0 成功
		返回 -1 图片加载失败
 ************************************************************************/
MYLIBDLL int RoateAndExtatBillImageEx(const char* srcName, const char* dstName,bool FlagWebData=false);


/************************************************************************
 * 描述:票据自动旋转并切割黑边(线程切边，异步;对A4进行旋转处理)
 * 输入:
		*srcName 源图像全路径名称 如：d:\1.jpg
        *dstName 切出的票据路径
        *
 * 输出
        返回 0 成功
		返回 -1 图片加载失败
 ************************************************************************/
MYLIBDLL int RoateAndExtatBillImage90(const char* srcName, const char* dstName);

//MYLIBDLL IplImage* rotateImage1(IplImage* img,int degree);
/************************************************************************
 * 描述:票据自动旋转并切割黑边（对A4进行旋转处理）
 * 输入:
		*srcName 源图像全路径名称 如：d:\1.jpg
        *dstName 切出的票据路径
 * 输出
        返回 0 成功
		返回 -1 图片加载失败
 ************************************************************************/
MYLIBDLL int NewRoateAndExtatBillImage(const char* srcName, const char* dstName);

MYLIBDLL int STRoateAndExtatBillImageEx(const char* srcName, const char* dstName);
/************************************************************************
 * 描述:定式票据切割
 * 输入:
*srcName 源图像全路径名称 如：d:\1.jpg
        *dstName 切出的票据路径
        *RoatFlag 图片是否需要旋转90°
 * 输出
        返回 0 成功
返回 -1 图片加载失败
 ************************************************************************/
MYLIBDLL int NewRoateAndExtatBillImageEx(const char* srcName, const char* dstName,bool FlagCase=false);

/************************************************************************
 * 描述:多边形截图
 * 输入:
		*src_file_name 源图像全路径名称 如：d:\1.jpg
        *dst_file_name 切出的票据路径
        *CPoint *SquePoint 多边形坐标数组
		*int MaxNum 指针数组的成员数
 * 输出
        返回 0 成功
		返回 -1 图片加载失败
 *备注 坐标数组请按顺序存储
 ************************************************************************/
MYLIBDLL int Arbitrary_cutting(const char* src_file_name,const char* dst_file_name,const CPoint *SquePoint,int MaxNum);


/************************************************************************
 * 描述:图片格式转换
 * 输入:
		*src_file_name 源图像全路径名称 如：d:\1.jpg
        *dst_file_name 输出图片路径，如：C:\\ss.bmp

 * 输出
        返回 0 成功
		返回 -1 图片加载失败
 *备注 
		目前支持的图像格式包括：
		Windows位图文件 - BMP, DIB；
		JPEG文件 - JPEG, JPG, JPE；
		便携式网络图片 - PNG；
		便携式图像格式 - PBM，PGM，PPM；
		Sun rasters - SR，RAS；
		TIFF文件 - TIFF，TIF;
		OpenEXR HDR 图片 - EXR;
		JPEG 2000 图片- jp2
 ************************************************************************/
MYLIBDLL int ImageConverter(const char* src_file_name,const char* dst_file_name);

/**********************************************************************
 * 检测票据是否符合某个版式,暂只支持转账支票、支付凭证和电汇凭证
 * 输入:
		*src_file_name:输入待确认文件名
		*template_type:模板类型名
		*compare_rate:匹配率,如果值大于0.8则直接输出,不进行反向检测,
		              一般而言,输出值低于0.8即可认为版式不正确.....
		*orient_flag:朝向标记,0-ok,1-需要180度旋转
		*template_path:模板路径
 * 输出:
		* 0-OK
		*-1-ERROR,源图像路径不存在
		*-2-ERROR,传入了不支持的模板类型
		*-3-ERROR,读取模板错误
 *
 **********************************************************************/
MYLIBDLL int AutoIdentify(const char* src_file_name,int *modelID,int *or_flag,char *modelname);

MYLIBDLL int AutoIdentifyEx(const char* src_file_name,int *modelID,int *or_flag,char *modelname,bool FlagWebData=false);


/**********************************************************************
 * 检测票据是否符合某个版式,读取xml文件配置
 * 输入:
		*src_file_name:输入待确认文件名
		*ResultData:输出匹配信息(模板名_模板ID_匹配率)；eg.测试票据_1_0.801
		*FlagWebData:是否为网络图片,默认false(本地图片)
 * 输出:
		* 0-OK
		*-1-ERROR,源图像路径不存在
		*-2-ERROR,传入了不支持的模板类型
		*-3-ERROR,读取模板错误
		*-8-ERROR,xml文件读取失败
		*-4-ERROR,所有模板识别率低于0.1
 *
 **********************************************************************/
MYLIBDLL int NewAutoIdentifyEx(const char* src_file_name,char *ResultData,bool FlagWebData);
MYLIBDLL int NewAutoIdentifyEx2(const char* src_file_name,char *ResultData,bool FlagWebData);
MYLIBDLL int STAutoIdentifyEx(const char* src_file_name,char *ResultData);
MYLIBDLL int STAutoIdentifyAndFlag(const char* src_file_name,char *ResultData,char * iflag);
/**********************************************************************
 * 简单对2张票据进行比较，输出相似率
 * 输入:
		*object_file_name:输入文件名
		*object_file_name1:输入文件名1
		*CompareEve:匹配率,如果值大于0.8则可认为相同

 * 输出:
		* 0-OK
		*-1-ERROR,源图像路径不存在
		*-2-ERROR,传入了不支持的模板类型
		*-3-ERROR,读取模板错误
 *
 **********************************************************************/
MYLIBDLL int AutoCompare(const char* object_file_name,const char* object_file_name1,double *CompareEve);

/**********************************************************************
 * 对票据指定区域进行识别
 * 输入:
		*src_file_name:输入票据文件名
		*PointString:识别区域左上方点+宽高（eg.100,100,200,200@）
		*OCRresult:区域识别结果

 * 输出:
		* 0-OK
		*-1-ERROR,源图像路径不存在
		*-2-ERROR,传入了不支持的模板类型
		*-3-ERROR,读取模板错误
 *
 **********************************************************************/
MYLIBDLL int DllImageAreaOCR(const char* src_file_name,int SectionID,int PartionID,char* OCRresult,bool FlagWebData=false);

MYLIBDLL int DllImageAreaOCREx(const char* src_file_name,const char* PointString,char* OCRresult,bool FlagWebData=false);

MYLIBDLL int  STImageToPointString(char* src_file_name,char* PointString);

/**********************************************************************
 * 返回DLL相关信息
 * 输入:
		*ActiveXData:返回信息

 * 输出:
		* 0-OK
 *
 **********************************************************************/
MYLIBDLL int HaveActiveXData(char *ActiveXData);

/**********************************************************************
 * 简单纸张识别
 * 输入:
		*SrcImagePath:图片信息

 * 输出:
		* *resultFlag，0不是纸，1是纸
 * 返回:
		* -1:图片读取失败；-2:传入路径不正确；0-OK
 **********************************************************************/
MYLIBDLL int PaperReconfig(char *SrcImagePath,int *resultFlag);

/**********************************************************************
 * 判断票据摆放方向
 * 输入:
		*ImagePath:图片信息

 * 输出:
		* *JudgeResult，0 竖放，1 横放
 * 返回:
		* -1:图片读取失败；-2:传入路径不正确；0-OK
 **********************************************************************/
MYLIBDLL int PaperDirectJudge(char* ImagePath,int* JudgeResult);


MYLIBDLL int TakePicture(char* szImageFile,int nWidth,int nHight,int CamraID);

/**********************************************************************
 * 摄像头拍照接口（最多支持3个同时拍照）
 * 输入:
 * 输出:
		* 0-OK
		*-1-ERROR 打开设备失败
		*-2-ERROR 帧为空
		*-3-ERROR 输入非法
 *
 **********************************************************************/
MYLIBDLL int OpenCmareaEx(int CamraNums,char *ConfigFile);
MYLIBDLL int OpenCmareaExID(int CamraNums,const char *ConfigFile);
MYLIBDLL int TakePictreEx(int CameraID,const char *ImageName);

//HavePictreEx();输入配置编号ID，Gdiplus::Bitmap指针，输出Gdiplus::Bitmap信息
MYLIBDLL int HavePictreEx(int CameraID,Gdiplus::Bitmap* &pCBitmap);
MYLIBDLL int STOpenCmareaEx(byte CameraID);
MYLIBDLL HBITMAP STHavePictreEx(byte CameraID);
MYLIBDLL int STReleaseBitMap(HBITMAP hbittmap);
MYLIBDLL int STCloseCameraId(byte CameraID);
/**********************************************************************
*HavingUcharEncode();输入配置编号ID，char*指针，输出JPG文件流信息，流大小
*UcharEncodeRelease;释放JPG文件流指针
*调用方式：
	open*
	unsigned char *databuf=NULL;
	long length=0;
	HavingUcharEncode(1,&databuf,&length);
	....
	UcharEncodeRelease(&databuf);
	close*
 **********************************************************************/
MYLIBDLL int HavingUcharEncode(int CameraID,unsigned char** EncodeBuf,long *ByeLength);
MYLIBDLL int UcharEncodeRelease(unsigned char** EncodeBuf);
MYLIBDLL int CloseCameraEx();
MYLIBDLL int CloseCameraId(int camearaid);


/*******************全局回调函数**************************************
//MyCallBack 函数返回值：unsigned int uMsg,char* lParam
*****Case1:lParam为空,此时为视频侦测回调信息
			返回值：0xO3,摄像头打开失败;0xO4,摄像头分辨率与配置不一致;
			0xO5,敏感区域MASK未设置失败;0x06,侦测报警;0x07,视频遮挡
*****Case2:lParam为"CP"，此时为录像回调信息
			返回值：0xO1,摄像头打开失败;0xO2,视频文件创建失败;
			0x03,录像配置信息不正确;0x04,帧读取失败;0x05,帧写入失败
*****Case2:lParam为"MOTCAP"，此时为视频侦测并录像回调信息
			返回值：0xO2,视频文件创建失败;0xO3,摄像头打开失败;0xO4,摄像头分辨率与配置不一致;
			0xO5,敏感区域MASK未设置失败;0x06,侦测报警;0x07,视频遮挡		
***********************************************************************/
MYLIBDLL int MotionDe_CallBack( MotionDetectRev MyCallBack );



//移动侦测相关functions
/**********************************************************************
 * 开启视频侦测
 * 输入:*DetectConfigPath 配置文件目录(eg.C://PIC)
 * 输出:
		* 0-OK
 *
 **********************************************************************/
MYLIBDLL int Begin_Motion_Detect(char *DetectConfigPath);

/**********************************************************************
 * 结束视频侦测
 * 输入:
 * 输出:
		* 0-OK
 *
 **********************************************************************/
MYLIBDLL int End_Motion_Detect();

/**********************************************************************
 * 配置视频侦测摄像头遮挡模型
 * 输入:*SrcImage 模型原图 *ConfigPath 模型配置文件目录(eg.C://PIC)
 * 输出:
		* 0-OK
 *
 **********************************************************************/
MYLIBDLL int ConfigHashImage(char* SrcImage,char* ConfigPath);

/**********************************************************************
 * 开启录像线程
 * 输入:* CamraNums CameraConfig.ini配置中的[CameraNum*]编号+1
		* *ConfigFile CameraConfig.ini配置绝对路径
		* *CaptureName 录制视频全路径名称 eg.c://pic//out.avi
		* NoCompress 视频文件夹是否压缩，默认为压缩XVID，ps.需要安装XVID编码器
 * 输出:
		* 0-OK
 *
 **********************************************************************/
MYLIBDLL int CameraCapture(int CamraNums,char *ConfigFile,char* CaptureName,bool NoCompress=false);

/**********************************************************************
 * 停止录像线程
 * 输入:
 * 输出:
		* 0-OK
 *
 **********************************************************************/
MYLIBDLL int End_CameraCapture();


/**********************************************************************
 * 开启移动侦测并录像线程
 * 输入:* *DetectConfigPath CameraConfig.ini配置绝对路径
		* *CaptureName 录制视频全路径名称 eg.c://pic//out.avi
		* NoCompress 视频文件夹是否压缩，默认为压缩XVID，ps.需要安装XVID编码器
 * 输出:
		* 0-OK
 *
 **********************************************************************/
MYLIBDLL int Begin_Motion_Capture(char *DetectConfigPath,char* CaptureName,bool NoCompress=false);


/**********************************************************************
 * 停止移动侦测并录像线程
 * 输入:
 * 输出:
		* 0-OK
 *
 **********************************************************************/
MYLIBDLL int End_Motion_Capture();

/**********************************************************************
 * 停止移动侦测并录像线程
 * 输入:*SrcName1 *SrcName2 待比较的图片名（+路径）
 * 输出:
		* 返回值：比较结果 ret;
		*		  -1，图片读取失败 
		*		  -2，图片尺度差距大于10
		*		  1-64的整数，比较值(暂定: ret<=5 图片非常相似 5<ret<=10 图片大部分相似 ret>=10图片不相似)
 *
 **********************************************************************/
MYLIBDLL int CompareImageSimilar(const char* SrcName1,const char* SrcName2);

/**********************************************************************
 * 调节摄像头属性信息
 * 输入:*int CamraID 摄像头ID
		*int brightness  亮 度
		*int constrast  对比度
		*int hue  色 调
		*int saturation  饱和度
		*int sharpness  清晰度
		*int whitebalance  白平衡
		*int gain  曝光值
 * 输出:
		* 返回值：0 成功,-1 摄像头ID为负数，-2 设置失败;
 * 使用说明：
		* 该接口传入参数和amcap软件里面:选项->视频扑捉过滤器->视频Proc Amp菜单下参数设置一致；
		* 可以先用软件调节后，将配置参数传入
 * 注意事项:
		* 有的摄像头设置值在掉电后，会自动恢复默认值，针对这类摄像头在每次掉电插拔后需要重新设置一次
 **********************************************************************/
MYLIBDLL int ImageDll_SetVedioProperties(int CamraID,int brightness/*亮 度*/, int constrast/*对比度*/,
	int hue/*色 调*/, int saturation/*饱和度*/, int sharpness/*清晰度*/,
	int whitebalance/*白平衡*/, int gain/*曝光值*/);

/**********************************************************************
 * 任意角度寻找盖章点
 * 输入:
		*Imagefile 原图片全路径 
		*StamX，StamY 相对票据中心点在图片中的相对位置；PS：不能等于0
		*DirectFlag 图片切边后票据识别后，图片方向（true 正向，false 逆向）
 * 输出:
		*CPoint* StamPoin 返回盖章点
		*double *StamAngel 返回章面顺时针旋转角度
		* 返回值：比较结果 ret;
		*		  -1，输入图片路径为空 
		*		  -2，图片读取失败

 *
 **********************************************************************/
MYLIBDLL int SeachSealPoint(char* Imagefile,int StamX,int StamY,CPoint* StamPoint,double *StamAngel,bool DirectFlag/*(true 正向，false 逆向)*/);
//MYLIBDLL int STSeachSealPoint(char* Imagefile,int StamX,int StamY,int* StamPointX,int* StamPointY,double *StamAngel,int inputangle);
/**********************************************************************
 * 生成电子印章图片 默认尺寸：96dpi 圆直径为42mm
 * 输入:
		*pathName 生成图片全路径名称
		*StampType 生成印章类型:1.圆形 2.椭圆 3.矩形
		*F_str 印章类容(eg."中国银行测试用章")
		*S_str 印章附文信息(eg."三泰测试章")
		*T_str 印章日期(eg."201602")
 * 输出:		
		* 返回值：比较结果 ret;
		*		  -1，输入图片路径为空 
		*		   0,OK;

 *
 **********************************************************************/
MYLIBDLL int DrawStamp(char* pathName,int StampType,char* F_str,char* S_str,char* T_str);

/**********************************************************************
 * 生成业务流水码图片 默认尺寸：96dpi下，长40mm，宽10mm
 * 输入:
		*pathName 生成图片全路径名称
		*StampType 生成印章类型:1.圆形 2.椭圆 3.矩形
		*Rectflag  图片是否有边框
		*F_str 流水码类容(eg."AB123456")
 * 输出:		
		* 返回值：比较结果 ret;
		*		  -1，输入图片路径为空 
		*		   0,OK;

 *
 **********************************************************************/
MYLIBDLL int DrawRectData(char* pathName,char* F_str,bool Rectflag);

#endif
