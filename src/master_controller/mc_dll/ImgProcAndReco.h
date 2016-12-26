/************************************************************************/ 
/* 
文件名称：ImgProcAndReco.h
功能介绍：图像处理与识别
版本：v1.00（初版）
作者：罗明海
日期：2016.07.21
版权: 三泰电子
*/
/************************************************************************/
#pragma once
#ifdef IMAGEPROCESS_EXPORTS
#define MYLIBDLL extern "C" _declspec(dllexport) 
#else
#define MYLIBDLL extern "C" _declspec(dllimport) 
#endif
#include "ImgReco.h"
#define  QFStamperOnOff		1
//*****************C/C++*******************************
MYLIBDLL DWORD	RecoModelTypeByImg(const char* in_src_img_name,char *out_model_type);
MYLIBDLL DWORD	RecoModelTypeAndAngleByImg(const char* in_src_img_name,char *out_model_type,double &outangle);
MYLIBDLL DWORD	RecoModelTypeAndAngleExByImg(const char* in_src_img_name,char *out_model_type,double &outangle);
MYLIBDLL DWORD	RecoModelTypeAndAngleExByImgAndModel(const char* in_src_img_name,char *in_model_type,char *out_model_type,double &outangle);
MYLIBDLL DWORD	RecoModelTypeAndAngleAndModelPointByImg(const char* in_src_img_name,char *out_model_type,
	double &outangle,int &x,int &y);
MYLIBDLL DWORD	RecoModelTypeAndAngleAndModelPointExByImg(const char* in_src_img_name,char *out_model_type,
	double &outangle,int &x,int &y);
MYLIBDLL DWORD	RecoModelTypeAndAngleAndModelPointExByImgAndModel(const char* in_src_img_name,char *in_model_type,char *out_model_type,
	double &outangle,int &x,int &y);
MYLIBDLL DWORD	RecoModelTypeAndCodeAndAngleAndPointByImg(const char* in_cut_img_name,char *in_model_type,char *out_model_type,
	char* out_Vocher_Number,char* out_trace_code,int &x,int &y,int &outangle );
MYLIBDLL DWORD  GetABCSealCheckStampPoint(const char* in_cut_img_name,int idx,int &x,int &y,int &outangle);
MYLIBDLL DWORD  GetABCSealCheckStampPointEx(const char* in_cut_img_name,int *x,int *y,int *outangle);
MYLIBDLL DWORD	CutImgEdge(const char* in_src_img_name,char *out_dest_img_name);
MYLIBDLL DWORD	CutImgEdgeEx(const char* in_src_img_name,char *out_dest_img_name);
MYLIBDLL DWORD	AdjustImgByRotate(const char* in_src_img_name,char *out_dest_img_name);
MYLIBDLL DWORD	CutImgByRectArea(const char* in_src_img_name,int x,int y,int width,int height,char *out_dest_img_name);
MYLIBDLL DWORD	GetLastCutImgAngle(double &out_angle);
MYLIBDLL DWORD  GetModelStampPointByImgModel(const char *in_model_type,int &x,int &y);
MYLIBDLL DWORD  GetModelOcrAreaByImgModel(const char *in_model_type,int &left,int &top,int &right,int &bottom);
MYLIBDLL DWORD  GetModelThresholdByImgModel(const char *in_model_type,double &outshreshold);
MYLIBDLL DWORD  GetModelVocherNumberAreaByImgModel(const char *in_model_type,int &left,int &top,int &right,int &bottom);
MYLIBDLL DWORD  SearchImgStampPoint(const char* in_src_img_name,int in_x,int in_y,double in_angle,
	int &out_x,int &out_y,int &out_angle);
MYLIBDLL DWORD   SearchImgStampPointEx(const char* in_src_img_name,int in_x,int in_y,double in_angle,
	int &out_x,int &out_y,double &out_angle);
MYLIBDLL DWORD	RecoImgRectArea(const char* in_dest_img_name,int left,int top,int right,int bottom,double threshold,char *ocrresult,FontLib lib= image);
MYLIBDLL DWORD	RecoImg(const char* in_dest_img_name,char *ocrresult,FontLib lib= image);
MYLIBDLL DWORD  ImgConvert(const char* in_src_img_name,const char* out_dest_img_name);
MYLIBDLL DWORD  SetImgDPI(const char* src_file_name,double rate_x,double rate_y,int dpi,int TypeFlag,
	const char* dst_file_name);
MYLIBDLL bool	IsExistsPaper(const char* src_file_name);
MYLIBDLL DWORD ResetImgSize(const char* src_file_name,int width,int height);
MYLIBDLL DWORD RotateImg(const char* src_file_name,double in_angle,const char* out_dest_img_name);
MYLIBDLL DWORD Merge2Imgs(const char* src_file_name1,const char* src_file_name2,const char* dst_file_name);
MYLIBDLL DWORD CalcImgRate(const char* src_file_name,int dpi,double &rate_x,double &rate_y);
//******************************C#*************************
MYLIBDLL int CS_RecoModelTypeByImg(const char* in_src_img_name,char *out_model_type);
MYLIBDLL int CS_RecoModelTypeAndAngleByImg(const char* in_src_img_name,char *out_model_type,char *outangle);
MYLIBDLL int CS_RecoModelTypeAndAngleExByImg(const char* in_src_img_name,char *out_model_type,char *outangle);
MYLIBDLL int CS_RecoModelTypeAndAngleAndModelPointByImg(const char* in_src_img_name,char *out_model_type,
	char *outangle,char *x,char *y);
MYLIBDLL int CS_RecoModelTypeAndAngleAndModelPointExByImg(const char* in_src_img_name,char *out_model_type,
	char *outangle,char *x,char *y);
MYLIBDLL int CS_RecoModelTypeAndCodeAndAngleAndPointByImg(const char* in_cut_img_name,const char *in_model_type,char *out_model_type,
	char* out_Vocher_Number,char* out_trace_code,char* x,char* y,char* outangle );
MYLIBDLL int CS_GetABCSealCheckStampPoint(const char* in_cut_img_name,char* idx,char* x,char* y,char* outangle);
MYLIBDLL int CS_GetABCSealCheckStampPointEx(const char* in_cut_img_name,char* outbuf);
MYLIBDLL int CS_GetModelThresholdByImgModel(const char *in_model_type,char* outshreshold);
MYLIBDLL int CS_CutImgEdge(const char* in_src_img_name,char *out_dest_img_name);
MYLIBDLL int CS_CutImgEdgeEx(const char* in_src_img_name,char *out_dest_img_name);
MYLIBDLL int CS_AdjustImgByRotate(const char* in_src_img_name,char *out_dest_img_name);
MYLIBDLL int CS_CutImgByRectArea(const char* in_src_img_name,char* x,char* y,char* width,char* height,char *out_dest_img_name);
MYLIBDLL int CS_GetLastCutImgAngle(char *out_angle);
MYLIBDLL int CS_GetModelStampPointByImgModel(const char *in_model_type,char *x,char *y);
MYLIBDLL int CS_GetModelOcrAreaByImgModel(const char *in_model_type,char *left,char *top,char *right,char *bottom);
MYLIBDLL int CS_GetModelVocherNumberAreaByImgModel(const char *in_model_type,char *left,char *top,char *right,char *bottom);
MYLIBDLL int CS_SearchImgStampPoint(const char* in_src_img_name,char *in_x,char *in_y,char *in_angle,
	char *out_x,char *out_y,char *out_angle);
MYLIBDLL int CS_SearchImgStampPointEx(const char* in_src_img_name,char *in_x,char *in_y,char *in_angle,
	char *out_x,char *out_y,char *out_angle);
MYLIBDLL int CS_RecoImgRectArea(const char* in_dest_img_name,char *left,char *top,char *right,char *bottom,char* threshold,char *ocrresult);
MYLIBDLL int CS_RecoImgRectAreaByLib(const char* in_dest_img_name,char *left,char *top,char *right,char *bottom,char* threshold,char *ocrresult,char *lib);
MYLIBDLL int CS_RecoImg(const char* in_dest_img_name,char *ocrresult);
MYLIBDLL int CS_ImgConvert(const char* in_src_img_name,const char* out_dest_img_name);
MYLIBDLL int CS_SetImgDPI(const char* src_file_name,char *rate_x,char *rate_y,char *dpi,char *TypeFlag,
	const char* dst_file_name);
MYLIBDLL int CS_ResetImgSize(const char* src_file_name,char *width,char *height);
MYLIBDLL int CS_RotateImg(const char* src_file_name,char *in_angle,const char* out_dest_img_name);
MYLIBDLL int CS_Merge2Imgs(const char* src_file_name1,const char* src_file_name2,const char* dst_file_name);
MYLIBDLL int CS_IsExistsPaper(const char* src_file_name);
MYLIBDLL int CS_CalcImgRate(const char* src_file_name,char* dpi,char* rate_x,char* rate_y);
