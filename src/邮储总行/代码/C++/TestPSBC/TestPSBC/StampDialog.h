#pragma once
#include "afxwin.h"

#include <map>

// CStampDialog 对话框

class CStampDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CStampDialog)

public:
	CStampDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CStampDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_STAMP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedbtnsetpagingseal();
	afx_msg void OnBnClickedbtnphotosensitive();
	afx_msg void OnBnClickedbtnmanustamp();
	CEdit m_mualX;
	CEdit m_mualY;
	CEdit m_mualAnlge;
	CEdit m_maulPrintNum;
	afx_msg void OnBnClickedbtnautostamp();
	afx_msg void OnBnClickedbtnlockprinter();
    afx_msg void OnBnSetCameraParam();
    afx_msg void OnComboChange();
	afx_msg void OnBnClickedbtnunlockprinter();
	afx_msg void OnBnClickedbtnlockstatus();
	afx_msg void OnBnClickedbtnopencam();
	afx_msg void OnBnClickedbtnclosecam();
	afx_msg void OnBnClickedbtngetimage();
	CEdit m_ImagePath;
	CEdit m_editType;
	CEdit m_borderPro;
	afx_msg void OnBnClickedbtnrotateimage();
	CEdit m_SrcImage;
	CEdit m_dstImage;
	CEdit m_RotateAngle;
	afx_msg void OnBnClickedbtncheckcamstatus();
	afx_msg void OnBnClickedbtnopenlockinfo();
	afx_msg void OnBnClickedbtndeletelockinfo();

    void ShowPromptInfo(int index);
	void ShowPromptInfo(int index,CString info);
	CEdit m_editOutput;
	
	CStatic m_imgeCtrl;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();

    std::map<CString, int>  camera_para_map_;
    CComboBox               camera_para_type_;
    CEdit                   camera_para_value_;
};
