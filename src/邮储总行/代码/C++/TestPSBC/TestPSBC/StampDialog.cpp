// StampDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "TestPSBC.h"
#include "StampDialog.h"
#include "afxdialogex.h"
#include "psbctype.h"


// CStampDialog 对话框

IMPLEMENT_DYNAMIC(CStampDialog, CDialogEx)

CStampDialog::CStampDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CStampDialog::IDD, pParent)
	
{

}

CStampDialog::~CStampDialog()
{
}

void CStampDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, mualX, m_mualX);
	DDX_Control(pDX, mualY, m_mualY);
	DDX_Control(pDX, mualAngle, m_mualAnlge);
	DDX_Control(pDX, maulPrintNum, m_maulPrintNum);
	DDX_Control(pDX, editImagePath, m_ImagePath);
	DDX_Control(pDX, editType, m_editType);
	DDX_Control(pDX, editBorderPro, m_borderPro);
	DDX_Control(pDX, btnSrcImage, m_SrcImage);
	DDX_Control(pDX, btnDstImage, m_dstImage);
	DDX_Control(pDX, btnRotateAngle, m_RotateAngle);
	DDX_Control(pDX, eidtOutPut, m_editOutput);
	DDX_Control(pDX, ImgPicCtl, m_imgeCtrl);
    DDX_Control(pDX, IDC_COMBO_CAMERA_PARA, camera_para_type_);
    DDX_Control(pDX, IDC_EDIT_PARA_VALUE, camera_para_value_);
}


BEGIN_MESSAGE_MAP(CStampDialog, CDialogEx)
	ON_BN_CLICKED(btnSetPagingSeal, &CStampDialog::OnBnClickedbtnsetpagingseal)
	ON_BN_CLICKED(btnPhotoSensitive, &CStampDialog::OnBnClickedbtnphotosensitive)
	ON_BN_CLICKED(btnManuStamp, &CStampDialog::OnBnClickedbtnmanustamp)
	ON_BN_CLICKED(btnAutoStamp, &CStampDialog::OnBnClickedbtnautostamp)
	ON_BN_CLICKED(btnLockPrinter, &CStampDialog::OnBnClickedbtnlockprinter)
    ON_BN_CLICKED(IDC_BUTTON_SET_CAMERA, &CStampDialog::OnBnSetCameraParam)
	ON_BN_CLICKED(btnunLockPrinter, &CStampDialog::OnBnClickedbtnunlockprinter)
	ON_BN_CLICKED(btnLockStatus, &CStampDialog::OnBnClickedbtnlockstatus)
	ON_BN_CLICKED(btnOpenCam, &CStampDialog::OnBnClickedbtnopencam)
	ON_BN_CLICKED(btnCloseCam, &CStampDialog::OnBnClickedbtnclosecam)
	ON_BN_CLICKED(btnGetImage, &CStampDialog::OnBnClickedbtngetimage)
	ON_BN_CLICKED(btnRotateImage, &CStampDialog::OnBnClickedbtnrotateimage)
	ON_BN_CLICKED(btnCheckCamStatus, &CStampDialog::OnBnClickedbtncheckcamstatus)
	ON_BN_CLICKED(btnOpenLockInfo, &CStampDialog::OnBnClickedbtnopenlockinfo)
    ON_BN_CLICKED(btnDeleteLockInfo, &CStampDialog::OnBnClickedbtndeletelockinfo)
    ON_CBN_SELCHANGE(IDC_COMBO_CAMERA_PARA, &CStampDialog::OnComboChange)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CStampDialog 消息处理程序


void CStampDialog::OnBnClickedbtnsetpagingseal()
{
	// TODO: 在此添加控件通知处理程序代码
	int result = psbcStampApi::Instance()->setAcrossPageSeal() == 0 ? 0 : -1;
	ShowPromptInfo(result);
}

void CStampDialog::OnBnClickedbtnphotosensitive()
{
	CString _cx;
	CString _cy;
	CString _cangle;
	CString _cPrintNo;
	int _x=0;
	int _y=0;
	int _angle=0;
	int _PrintNo;

	m_mualX.GetWindowText(_cx);
    if (_cx.IsEmpty()) {
        MessageBox("请输入用印X参数");
        return;
    }

	m_mualY.GetWindowText(_cy);
    if (_cy.IsEmpty()) {
        MessageBox("请输入用印Y参数");
        return;
    }

	m_mualAnlge.GetWindowText(_cangle);
	m_maulPrintNum.GetWindowText(_cPrintNo);

	_x     = atoi(_cx.GetBuffer(0));
	_y     = atoi(_cy.GetBuffer(0));
	_angle = atoi(_cangle.GetBuffer(0));
	_PrintNo = atoi(_cPrintNo.GetBuffer(0));

	int result = psbcStampApi::Instance()->manualPrintStartByLight(
        _PrintNo,
        _x,
        _y,
        _angle) == 0 ?0 : -1;
    ShowPromptInfo(result);
}

void CStampDialog::OnBnClickedbtnmanustamp()
{
    CString _cx;
	CString _cy;
	CString _cangle;
	CString _cPrintNo;
	int _x=0;
	int _y=0;
	int _angle=0;
	int _PrintNo;
	int result = 0;

	m_mualX.GetWindowText(_cx);
    if (_cx.IsEmpty()) {
        MessageBox("请输入用印X参数");
        return;
    }

	m_mualY.GetWindowText(_cy);
    if (_cy.IsEmpty()) {
        MessageBox("请输入用印Y参数");
        return;
    }

	m_mualAnlge.GetWindowText(_cangle);
	m_maulPrintNum.GetWindowText(_cPrintNo);

	_x     = atoi(_cx.GetBuffer(0));
	_y     = atoi(_cy.GetBuffer(0));
	_angle = atoi(_cangle.GetBuffer(0));
    _PrintNo = atoi(_cPrintNo.GetBuffer(0));

	result = psbcStampApi::Instance()->manualPrintStart(
        _PrintNo,
        _x,
        _y,
        _angle) == 0 ? 0 : -1;
    ShowPromptInfo(result);
}

void CStampDialog::OnBnClickedbtnautostamp()
{
    MessageBox("注意: 此时X,Y坐标输入框中值的单位是毫米(mm)");

    CString _cx;
	CString _cy;
	CString _cangle;
	CString _cPrintNo;
	int _x=0;
	int _y=0;
	int _angle=0;
	int _PrintNo;

	m_mualX.GetWindowText(_cx);
    if (_cx.IsEmpty()) {
        MessageBox("请输入用印X参数");
        return;
    }

	m_mualY.GetWindowText(_cy);
    if (_cy.IsEmpty()) {
        MessageBox("请输入用印Y参数");
        return;
    }

	m_mualAnlge.GetWindowText(_cangle);
	m_maulPrintNum.GetWindowText(_cPrintNo);

	_x     = atoi(_cx.GetBuffer(0));
	_y     = atoi(_cy.GetBuffer(0));
	_angle = atoi(_cangle.GetBuffer(0));
	_PrintNo = atoi(_cPrintNo.GetBuffer(0));

	int result = psbcStampApi::Instance()->autoPrintStart(
        _PrintNo,
        _x,
        _y,
        _angle) == 0 ? 0 : -1;
    ShowPromptInfo(result);
}

void CStampDialog::OnBnClickedbtnlockprinter()
{
	int result=0;
	result = psbcStampApi::Instance()->lockPrinter()==0?0:-1;
	ShowPromptInfo(result);
}

void CStampDialog::OnComboChange()
{
    int idx = camera_para_type_.GetCurSel();
    CString text;
    camera_para_type_.GetLBText(idx, text);

    CString val;
    camera_para_value_.GetWindowText(val);
    if (val.IsEmpty()) {
        MessageBox("请输入参数值");
        return;
    }

    int para_val = atoi(val.GetBuffer(val.GetLength()));
    val.ReleaseBuffer();

    camera_para_map_.insert(std::make_pair(text, para_val));

    MessageBox(text + " " + val);
}

void CStampDialog::OnBnSetCameraParam()
{
    if (camera_para_map_.size() < 7) {
        MessageBox("未对所有参数进行设置, 未设置参数将采取按值为0进行处理.");
    }

    int brightness = 0;
    int constrast = 0;
    int hue = 0;
    int saturation = 0;
    int sharpness = 0;
    int whitebalance = 0;
    int gain = 0;
    std::map<CString, int>::iterator it = camera_para_map_.begin();
    for (; it != camera_para_map_.end(); ++it) {
        if (it->first == "亮度") {
            brightness = it->second;
            continue;
        }

        if (it->first == "对比度") {
            constrast = it->second;
            continue;
        }

        if (it->first == "色调") {
            hue = it->second;
            continue;
        }

        if (it->first == "饱和度") {
            saturation = it->second;
            continue;
        }

        if (it->first == "清晰度") {
            sharpness = it->second;
            continue;
        }

        if (it->first == "白平衡") {
            whitebalance = it->second;
            continue;
        }

        if (it->first == "曝光值") {
            gain = it->second;
            continue;
        }
    }

    int ret = psbcStampApi::Instance()->setVedioProperties(
        brightness,
        constrast,
        hue,
        saturation,
        sharpness,
        whitebalance,
        gain);
    ShowPromptInfo(ret);
}

void CStampDialog::OnBnClickedbtnunlockprinter()
{
	int result=0;
	result = psbcStampApi::Instance()->unLockPrinter()==0?0:-1;
	ShowPromptInfo(result);
}

void CStampDialog::OnBnClickedbtnlockstatus()
{
    int result = psbcStampApi::Instance()->checkLockState();
    switch (result) {
    case 0:
        ShowPromptInfo(5);
        break;
    case -1:
        ShowPromptInfo(6);
        break;
    default:
        ShowPromptInfo(-1);
        break;
    }
}

void CStampDialog::OnBnClickedbtnopencam()
{
	int result = psbcStampApi::Instance()->openVideoCap()==0?9:10;
	ShowPromptInfo(result);
}

void CStampDialog::OnBnClickedbtnclosecam()
{
	int result = psbcStampApi::Instance()->closeVideoCap()==0?11:12;
	ShowPromptInfo(result);
}

// 拍照-获取图像
void CStampDialog::OnBnClickedbtngetimage()
{
    // 存图路径
    CString _cPath;
	m_ImagePath.GetWindowText(_cPath);
    if (_cPath.IsEmpty()) {
        MessageBox("请输入图片存放路径");
        return;
    }

    char* szPath = _cPath.GetBuffer(_cPath.GetLength());
    _cPath.ReleaseBuffer();

    // 图片格式
    CString _cType;
	m_editType.GetWindowText(_cType);
    if (_cType.IsEmpty()) {
        MessageBox("请按要求输入图片格式");
        return;
    }

    char* type_str = _cType.GetBuffer(_cType.GetLength());
    int type = atoi(type_str);
    if (type != 0 && type != 1) {
        MessageBox("不支持的图片格式, 请重新输入");
        return;
    }

    // 是否切边
    CString _cbp;
	m_borderPro.GetWindowText(_cbp);
    if (_cbp.IsEmpty()) {
        MessageBox("请指定是否进行图像处理");
        return;
    }

    char* erase_str = _cbp.GetBuffer(_cbp.GetLength());
    _cbp.ReleaseBuffer();
    int bp = atoi(erase_str);
    if (bp != 0 && bp != 1) {
        MessageBox("请指定是否进行图像处理");
        return;
    }

	int result = psbcStampApi::Instance()->getImageFormat(
        (const char *)szPath,
        type,
        bp) ==0 ? 0 : -1;
    if (0 != result) {
        MessageBox("拍照失败");
        return;
    }

	// 获取图片的宽 高度  
	UpdateWindow();
	int height, width;
	CRect rect;//定义矩形类
	CRect rect1;
	CImage image; //创建图片类
	//根据路径载入图片  
	if(image.Load((char*)szPath)==0) {
		height = image.GetHeight();
		width = image.GetWidth();
		m_imgeCtrl.GetClientRect(&rect); //获得pictrue控件所在的矩形区域
		CDC *pDc = m_imgeCtrl.GetDC();//获得pictrue控件的Dc
		SetStretchBltMode(pDc->m_hDC,STRETCH_HALFTONE); 
		image.StretchBlt(pDc->m_hDC,rect,SRCCOPY);
		ReleaseDC(pDc);//释放picture控件的Dc
	}
}

void CStampDialog::OnBnClickedbtnrotateimage()
{
	CString _csrcPath;
	CString _cdestPath;
	CString _cangle;

	int result = 0;
	CImage  image;  
 
	m_SrcImage.GetWindowText(_csrcPath);
    if (_csrcPath.IsEmpty()) {
        MessageBox("请输入源图路径");
        return;
    }

    char* szSrcPath = _csrcPath.GetBuffer(_csrcPath.GetLength());
    _csrcPath.ReleaseBuffer();

	m_dstImage.GetWindowText(_cdestPath);
    if (_cdestPath.IsEmpty()) {
        MessageBox("请输入目标图片路径");
        return;
    }

    char* szDstPath = _cdestPath.GetBuffer(_cdestPath.GetLength());
    _cdestPath.ReleaseBuffer();

	m_RotateAngle.GetWindowText(_cangle);
    if (_cangle.IsEmpty()) {
        MessageBox("请指定旋转角度");
        return;
    }

	int cangle = atoi(_cangle.GetBuffer(_cangle.GetLength()));
    _cangle.ReleaseBuffer();

    // 根据路径载入图片
	result = psbcStampApi::Instance()->revolveImg(
        (const char*)szSrcPath,
        (const char*)szDstPath,
        cangle) == 0 ? 0 : -1;
    if (0 != result) {
        MessageBox("图片旋转失败");
        return;
    }

	int height, width;
	CRect rect;//定义矩形类
	CRect rect1;
	//根据路径载入图片  
	if(image.Load((char*)szDstPath)==0)
	{
		height = image.GetHeight();
		width = image.GetWidth();
		m_imgeCtrl.GetClientRect(&rect); //获得pictrue控件所在的矩形区域
		CDC *pDc = m_imgeCtrl.GetDC();//获得pictrue控件的Dc
		SetStretchBltMode(pDc->m_hDC,STRETCH_HALFTONE); 
		image.StretchBlt(pDc->m_hDC,rect,SRCCOPY);
		ReleaseDC(pDc);//释放picture控件的Dc
	}
}

void CStampDialog::OnBnClickedbtncheckcamstatus()
{
    int result = psbcStampApi::Instance()->checkVideoState();
    char buf[512] = { 0 };
    sprintf(buf, "当前摄像头%s", result == 1 ? "打开" : "关闭");
    m_editOutput.Clear();
    m_editOutput.SetWindowTextA(buf);
}

void CStampDialog::OnBnClickedbtnopenlockinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	char* pLockInof = psbcStampApi::Instance()->readOpenBackDoorExceptionInfo();
    char buf[512] = { 0 };
    sprintf(buf, "所有异常开锁记录: %s", pLockInof);

    m_editOutput.Clear();
    m_editOutput.SetWindowTextA(buf);
}

void CStampDialog::OnBnClickedbtndeletelockinfo()
{
	int result = psbcStampApi::Instance()->delOpenBackDoorExceptionInfo();
    char buf[512] = { 0 };
    sprintf(buf, "清空异常开锁信息记录%s", result == 0? "成功" : "失败");

    m_editOutput.Clear();
    m_editOutput.SetWindowTextA(buf);
}

void CStampDialog::ShowPromptInfo(int index)
{
	CString tmpString("执行结果:");
	tmpString.AppendFormat("%s",psbcPromptInfo::Instance()->_mapPromptInfo[index].c_str());
	m_editOutput.SetWindowTextA(tmpString.GetBuffer(0));
}

void CStampDialog::ShowPromptInfo(int index,CString info)
{
	CString tmpString("执行结果:");
	tmpString.AppendFormat("%s",psbcPromptInfo::Instance()->_mapPromptInfo[index].c_str());
	tmpString.Append("   输出信息:");
	tmpString.Append(info);
	m_editOutput.SetWindowTextA(tmpString.GetBuffer(0));
}

HBRUSH CStampDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd == this)
	{
		CBrush m_brBk;
		return m_brBk;

	}

	return hbr;

}


BOOL CStampDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    // 拍照参数
	m_ImagePath.SetWindowTextA("C:\\reImag.jpg");
	m_editType.SetWindowTextA("1");
	m_borderPro.SetWindowTextA("1");

	m_SrcImage.SetWindowTextA("C:\\reImag.jpg");
	m_dstImage.SetWindowTextA("C:\\dstImag.jpg");
	m_RotateAngle.SetWindowTextA("90");

    // 默认用印参数
    m_mualAnlge.SetWindowTextA("0");
    m_maulPrintNum.SetWindowTextA("1");

    // 设置摄像头属性
    camera_para_map_.clear();
    camera_para_type_.Clear();
    camera_para_type_.AddString("亮度");
    camera_para_type_.AddString("对比度");
    camera_para_type_.AddString("色调");
    camera_para_type_.AddString("饱和度");
    camera_para_type_.AddString("清晰度");
    camera_para_type_.AddString("白平衡");
    camera_para_type_.AddString("曝光值");
    camera_para_type_.SetCurSel(camera_para_type_.AddString("先输入参数值再切换对应参数类型"));

    m_mualX.SetWindowTextA("1000");
    m_mualY.SetWindowTextA("600");

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
