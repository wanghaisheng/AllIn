#pragma once
#include "psbctype.h"
#include "afxwin.h"

// CBasicFunDialog 对话框

class CBasicFunDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CBasicFunDialog)

public:
	CBasicFunDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CBasicFunDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_BASIC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedbtnopencon();
	afx_msg void OnBnClickedbtninitmachie();
private:
	void ShowPromptInfo(int index);
	void ShowPromptInfo(int index,CString info);

public:
	CEdit m_editInfo;
	afx_msg void OnBnClickedbtngetstampinfo();
	afx_msg void OnBnClickedbtnsetslotinfo();
	CEdit m_machineNum;
	CEdit m_slotStampId;
	afx_msg void OnBnClickedbtnsetstampinfo();
	CEdit m_editDeviceNum;
	afx_msg void OnBnClickedbtnsetslotinfo3();
	afx_msg void OnBnClickedbtnopenpagedoor();
	afx_msg void OnBnClickedbtncheckstamp();
	CEdit m_editPx;
	CEdit m_editPy;
	CEdit m_PAngle;
	afx_msg void OnBnClickedbtneopensafedoor();
	afx_msg void OnBnClickedbtnopensafedoor();
	CEdit m_eOpenSafeDoor;
	afx_msg void OnBnClickedbtngetdevicenum();
	afx_msg void OnBnClickedbtngetdevicetypenum();
	afx_msg void OnBnClickedbtngetpdoorstatus();
	afx_msg void OnBnClickedbtnopencamlight();
	afx_msg void OnBnClickedbtnclosecamlight();
	afx_msg void OnBnClickedbtngeterrorinfo();
	CEdit m_editNo;
	CEdit m_editSerialNo;
	afx_msg void OnBnClickedbtnclosecon();
	afx_msg void OnBnClickedbtncheckcon();
	CEdit m_sealEditInfo;
	CButton m_slotInfo;
	afx_msg void OnBnClickedbtnslotinfo();
	CEdit m_slotStampIde;
	virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedbtnobtainslotinfo();
    afx_msg void OnBnClickedbtngetsdoorlockstatus();
};
