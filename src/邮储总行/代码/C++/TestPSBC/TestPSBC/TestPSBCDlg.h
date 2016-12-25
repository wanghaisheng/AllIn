
// TestPSBCDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"

#include "PSBCDialog.h"
#include "BasicFunDialog.h"
#include "StampDialog.h"


// CTestPSBCDlg 对话框
class CTestPSBCDlg : public CDialogEx
{
// 构造
public:
	CTestPSBCDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TESTPSBC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_psbcTab;
	CStampDialog m_stampDialog;
	CBasicFunDialog m_basicFunDialog;
	afx_msg void OnTcnSelchangeTabPsbc(NMHDR *pNMHDR, LRESULT *pResult);
};
