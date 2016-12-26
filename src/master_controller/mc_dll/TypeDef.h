#pragma once
#include "StdAfx.h"


#define MODEL_TYPE_LEN_MAX	128
#define ImgMaxSizeDiff	0.3
typedef struct _ModelHead             /* 模板头*/ 
{
	int ModelId;
	char ModelType[128];
	int Width;
	int Height;
	_ModelHead()
	{
		ModelId = 0;
		memset(ModelType,0,sizeof(ModelType));
		Width = 0;
		Height = 0;
	}
	_ModelHead &operator =(const _ModelHead &_modelhead)
	{
		if (this != &_modelhead)
		{
			this->ModelId = _modelhead.ModelId;
			memcpy(this->ModelType,_modelhead.ModelType,sizeof(_modelhead.ModelType));
			this->Width = _modelhead.Width;
			this->Height = _modelhead.Height;
		}
		return *this;
	}
}ModelHead , *LPModelHead;
//矩形区域
typedef struct _RectArea
{
	int Left;
	int Top;
	int Right;
	int Bottom;
	_RectArea()
	{
		Left = 0;
		Top = 0;
		Right = 0;
		Bottom = 0;
	}
	_RectArea &operator =(const _RectArea &_rectarea)
	{
		if (this != &_rectarea)
		{
			this->Left = _rectarea.Left; 
			this->Top = _rectarea.Top;
			this->Right = _rectarea.Right;
			this->Bottom = _rectarea.Bottom;
		}
		return *this;
	}
}RectArea , *LPRectArea;

//用印点
typedef struct _StampPoint
{
	int X;
	int Y;
	int R;
	_StampPoint()
	{
		X = 0;
		Y = 0;
		R = 0;
	}
	_StampPoint &operator =(const _StampPoint &_stamppoint)
	{
		if (this != &_stamppoint)
		{
			this->X = _stamppoint.X;
			this->Y = _stamppoint.Y;
			this->R = _stamppoint.R;
		}
		return *this;
	}
}StampPoint,*LPStampPoint;

//农行用印点检测
typedef struct _SCPoint
{
	int no;
	StampPoint spoint;
	_SCPoint()
	{
		no = 0;
	}
	_SCPoint &operator =(const _SCPoint &_scpoint)
	{
		if (this != &_scpoint)
		{
			this->no = _scpoint.no;

			this->spoint.X = _scpoint.spoint.X;
			this->spoint.Y = _scpoint.spoint.Y;
			this->spoint.R = _scpoint.spoint.R;
		}
		return *this;
	}
}SCPoint,*LPSCPoint;
//模板
typedef struct _ModelTemplate
{
	ModelHead m_modelhead;	//模板头
	RectArea m_featurearea;	//表头/特征区域
	RectArea m_featurearea2;//表头/特征区域2
	RectArea m_vochernumber; //凭证编号
	StampPoint m_stamppoint; //用印点
	RectArea m_ocrarea;	//识别区域
	double d_threshold;	//二值化值
	_ModelTemplate():d_threshold(0)
	{		
	}
	_ModelTemplate &operator =(const _ModelTemplate &_modeltemplate)
	{
		if (this != &_modeltemplate)
		{
			this->m_modelhead = _modeltemplate.m_modelhead;
			this->m_featurearea = _modeltemplate.m_featurearea;
			this->m_featurearea2 = _modeltemplate.m_featurearea2;
			this->m_vochernumber = _modeltemplate.m_vochernumber;
			this->m_stamppoint = _modeltemplate.m_stamppoint;
			this->m_ocrarea = _modeltemplate.m_ocrarea;
			this->d_threshold = _modeltemplate.d_threshold;
		}
		return *this;
	}
}ModelTemplate,*LPModelTemplate;