// BasicFunDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "TestPSBC.h"
#include "BasicFunDialog.h"
#include "afxdialogex.h"



// CBasicFunDialog 对话框

IMPLEMENT_DYNAMIC(CBasicFunDialog, CDialogEx)

CBasicFunDialog::CBasicFunDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBasicFunDialog::IDD, pParent)
{

}

CBasicFunDialog::~CBasicFunDialog()
{
}

void CBasicFunDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, editInfo, m_editInfo);
	DDX_Control(pDX, editMachineNum, m_machineNum);
	DDX_Control(pDX, editDeviceNum, m_editDeviceNum);
	DDX_Control(pDX, editPX, m_editPx);
	DDX_Control(pDX, editPY, m_editPy);
	DDX_Control(pDX, editPAngle, m_PAngle);
	DDX_Control(pDX, editAbSafeDoor, m_eOpenSafeDoor);
	DDX_Control(pDX, editErrorNo, m_editNo);
	DDX_Control(pDX, editSeialNo, m_editSerialNo);
	DDX_Control(pDX, editSealInfo, m_sealEditInfo);
	DDX_Control(pDX, btnSlotInfo, m_slotInfo);
	DDX_Control(pDX, editStampId, m_slotStampIde);
}


BEGIN_MESSAGE_MAP(CBasicFunDialog, CDialogEx)
	ON_BN_CLICKED(btnOpenCon, &CBasicFunDialog::OnBnClickedbtnopencon)
	ON_BN_CLICKED(btnInitMachie, &CBasicFunDialog::OnBnClickedbtninitmachie)
	ON_BN_CLICKED(btnGetStampInfo, &CBasicFunDialog::OnBnClickedbtngetstampinfo)
	ON_BN_CLICKED(btnSetSlotInfo, &CBasicFunDialog::OnBnClickedbtnsetslotinfo)
	ON_BN_CLICKED(btnSetStampInfo, &CBasicFunDialog::OnBnClickedbtnsetstampinfo)
	ON_BN_CLICKED(btnSetSlotInfo3, &CBasicFunDialog::OnBnClickedbtnsetslotinfo3)
	ON_BN_CLICKED(btnOpenPageDoor, &CBasicFunDialog::OnBnClickedbtnopenpagedoor)
	ON_BN_CLICKED(btnCheckStamp, &CBasicFunDialog::OnBnClickedbtncheckstamp)
	ON_BN_CLICKED(btnEOpenSafeDoor, &CBasicFunDialog::OnBnClickedbtneopensafedoor)
	ON_BN_CLICKED(btnOpenSafeDoor, &CBasicFunDialog::OnBnClickedbtnopensafedoor)
	ON_BN_CLICKED(btnGetDeviceNum, &CBasicFunDialog::OnBnClickedbtngetdevicenum)
	ON_BN_CLICKED(btnGetDeviceTypeNum, &CBasicFunDialog::OnBnClickedbtngetdevicetypenum)
	ON_BN_CLICKED(btnGetPDoorStatus, &CBasicFunDialog::OnBnClickedbtngetpdoorstatus)
	ON_BN_CLICKED(btnOpenCamLight, &CBasicFunDialog::OnBnClickedbtnopencamlight)
	ON_BN_CLICKED(btnCloseCamLight, &CBasicFunDialog::OnBnClickedbtnclosecamlight)
	ON_BN_CLICKED(btnGetErrorInfo, &CBasicFunDialog::OnBnClickedbtngeterrorinfo)
	ON_BN_CLICKED(btnCloseCon, &CBasicFunDialog::OnBnClickedbtnclosecon)
	ON_BN_CLICKED(btnCheckCon, &CBasicFunDialog::OnBnClickedbtncheckcon)
	ON_BN_CLICKED(btnSlotInfo, &CBasicFunDialog::OnBnClickedbtnslotinfo)
    ON_BN_CLICKED(btnGetSDoorLockStatus, &CBasicFunDialog::OnBnClickedbtngetsdoorlockstatus)
END_MESSAGE_MAP()


// CBasicFunDialog 消息处理程序


void CBasicFunDialog::OnBnClickedbtnopencon()
{
	CString cserail;
	int result =0;
	CString cResult;
	m_editSerialNo.GetWindowText(cserail);
    if (cserail.IsEmpty()) {
        MessageBox("请输入设备序列号");
        return;
    }

    char* serail = cserail.GetBuffer(cserail.GetLength());
    cserail.ReleaseBuffer();

	result=psbcStampApi::Instance()->connMachine(serail)==0?0:-1;
	ShowPromptInfo(result);
}

void CBasicFunDialog::OnBnClickedbtninitmachie()
{
	int result=0;
	result = psbcStampApi::Instance()->initializationMachine()==0?0:-1;
	ShowPromptInfo(result);
}

void CBasicFunDialog::ShowPromptInfo(int index)
{
    m_editInfo.Clear();

	CString tmpString("执行结果:");
	tmpString.AppendFormat("%s",psbcPromptInfo::Instance()->_mapPromptInfo[index].c_str());
	m_editInfo.SetWindowTextA(tmpString.GetBuffer(0));
}

void CBasicFunDialog::ShowPromptInfo(int index,CString info)
{
    m_editInfo.Clear();

	CString tmpString("执行结果:");
	tmpString.AppendFormat("%s",psbcPromptInfo::Instance()->_mapPromptInfo[index].c_str());
	tmpString.Append("   输出信息:");
	tmpString.Append(info);
	m_editInfo.SetWindowTextA(tmpString.GetBuffer(0));
}

void CBasicFunDialog::OnBnClickedbtngetstampinfo()
{
	// TODO: 在此添加控件通知处理程序代码

    // 获取机器码
    CString m_machinNumInfo;
    m_sealEditInfo.GetWindowText(m_machinNumInfo);
    if (m_machinNumInfo.IsEmpty()) {
        MessageBox("请输入查询设备编号");
        return;
    }

    char* machine_num = m_machinNumInfo.GetBuffer(m_machinNumInfo.GetLength());
    m_machinNumInfo.ReleaseBuffer();

    char *pinfoResult = psbcStampApi::Instance()->querySealInfo(machine_num);
	CString temp;
    if(pinfoResult != NULL)
	{
	    temp.AppendFormat("%s", pinfoResult);
	}
	else
    {
        temp.AppendFormat("查询信息失败:%s", pinfoResult);
	}

	ShowPromptInfo(0, temp.GetBuffer(temp.GetLength()));
}

void CBasicFunDialog::OnBnClickedbtnsetslotinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	 char infoResult[256]={0};
	 char mechineNum[24]={0};

	 char *pinfoResult = psbcStampApi::Instance()->querySlotInfo(mechineNum);
	 memcpy(infoResult,pinfoResult,sizeof(infoResult));

	 CString temp;
	 temp.AppendFormat("输出信息:%s",infoResult);
	 ShowPromptInfo(0,temp.GetBuffer(0));
	
}

void CBasicFunDialog::OnBnClickedbtnsetstampinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	CString machineNum;
	m_machineNum.GetWindowText(machineNum);
    if (machineNum.IsEmpty()) {
        MessageBox("请输入设备编号");
        return;
    }

    char* machineN = machineNum.GetBuffer(machineNum.GetLength());
    machineNum.ReleaseBuffer();

    CString stampId;
	m_slotStampIde.GetWindowText(stampId);
    if (stampId.IsEmpty()) {
        MessageBox("请输入章信息");
        return;
    }

    char* slotId = stampId.GetBuffer(stampId.GetLength());
    stampId.ReleaseBuffer();

    // 设置章信息
	int result =psbcStampApi::Instance()->initStamp(machineN, slotId) == 0 ? 0 : -1;
	ShowPromptInfo(result);
}

void CBasicFunDialog::OnBnClickedbtnsetslotinfo3()
{
	// TODO: 在此添加控件通知处理程序代码
	CString deviceNum;
	m_editDeviceNum.GetWindowText(deviceNum);
    if (deviceNum.IsEmpty()) {
        MessageBox("请输入设备编号");
        return;
    }

    char* _devnum = deviceNum.GetBuffer(deviceNum.GetLength());
    deviceNum.ReleaseBuffer();

	int result = psbcStampApi::Instance()->setMachineNum(_devnum)==0?0:-1;
	ShowPromptInfo(result);
}

void CBasicFunDialog::OnBnClickedbtnopenpagedoor()
{
	// TODO: 在此添加控件通知处理程序代码
    int result = psbcStampApi::Instance()->openPaperDoor() == 0 ? 0 : -1;
    ShowPromptInfo(result);
}


void CBasicFunDialog::OnBnClickedbtncheckstamp()
{
	// TODO: 在此添加控件通知处理程序代码
	CString _cx;
	CString _cy;
	CString _cangle;
	int _x=0;
	int _y=0;
	int _angle=0;

	m_editPx.GetWindowText(_cx);
    if (_cx.IsEmpty()) {
        MessageBox("请输入X坐标值");
        return;
    }

	m_editPy.GetWindowText(_cy);
    if (_cy.IsEmpty()) {
        MessageBox("请输入Y坐标值");
        return;
    }

	m_PAngle.GetWindowText(_cangle);
    if (_cangle.IsEmpty()) {
        MessageBox("请输入旋转角度值");
        return;
    }

	_x = atoi(_cx.GetBuffer(0));
	_y = atoi(_cy.GetBuffer(0));
	_angle = atoi(_cangle.GetBuffer(0));

	int result = psbcStampApi::Instance()->checkManualPrintPara(_x,_y,_angle)==0?0:-2;
	ShowPromptInfo(result);
}

void CBasicFunDialog::OnBnClickedbtneopensafedoor()
{
	CString abInfo;
	m_eOpenSafeDoor.GetWindowText(abInfo);
    if (abInfo.IsEmpty()) {
        MessageBox("请输入异常开锁信息");
        return;
    }

    char* ab_info = abInfo.GetBuffer(abInfo.GetLength());
    abInfo.ReleaseBuffer();

	int result = psbcStampApi::Instance()->openMachineBackDoorUnNormal(ab_info);
    if (0 == result)
	    ShowPromptInfo(result);
    else
        ShowPromptInfo(-1);
}

void CBasicFunDialog::OnBnClickedbtnopensafedoor()
{
	// TODO: 在此添加控件通知处理程序代码
	int result =0;
	result = psbcStampApi::Instance()->openMachineBackDoor();
	ShowPromptInfo(result);
}

void CBasicFunDialog::OnBnClickedbtngetdevicenum()
{
	char *presultInfo;
	CString cResult;
	presultInfo= psbcStampApi::Instance()->getMachineNum();
	cResult.AppendFormat("%s",presultInfo);
	ShowPromptInfo(0,cResult);
}

void CBasicFunDialog::OnBnClickedbtngetdevicetypenum()
{
	char *presultInfo;
	CString cResult;
	presultInfo= psbcStampApi::Instance()->getMachineType();
	cResult.AppendFormat("%s",presultInfo);
	ShowPromptInfo(0,cResult);
}

void CBasicFunDialog::OnBnClickedbtngetpdoorstatus()
{
    // 1-- - 未关闭, 2-- - 关闭
    int result = psbcStampApi::Instance()->checkPaperDoorState();
    switch (result) {
    case 1:
        ShowPromptInfo(result);
        break;
    case 2:
        ShowPromptInfo(result);
        break;
    default:
        MessageBox("获取状态失败");
        break;
    }
}

void CBasicFunDialog::OnBnClickedbtngetsdoorlockstatus()
{
    // TODO: Add your control notification handler code here
    // 1---未关闭, 2---关闭,
    int result = psbcStampApi::Instance()->checkBackDoorState();
    switch (result) {
    case 1:
        ShowPromptInfo(3);
        break;
    case 2:
        ShowPromptInfo(4);
        break;
    default:
        MessageBox("获取状态失败");
        break;
    }
}

void CBasicFunDialog::OnBnClickedbtnopencamlight()
{
	int result =psbcStampApi::Instance()->openVideoCapLight()==0?0:-1;
	ShowPromptInfo(result);
}

void CBasicFunDialog::OnBnClickedbtnclosecamlight()
{
	int result =psbcStampApi::Instance()->closeVideoCapLight()==0?0:-1;
	ShowPromptInfo(result);
}


void CBasicFunDialog::OnBnClickedbtngeterrorinfo()
{
	CString error;
	int errNo;
	CString cResult;
    m_editNo.GetWindowText(error);
    if (error.IsEmpty()) {
        MessageBox("请输入错误码");
        return;
    }

    std::string err_str = error.GetBuffer(error.GetLength());
    error.ReleaseBuffer();
    for (size_t i = 0; i < err_str.length(); ++i) {
        char ch = err_str.at(i);
        if (ch < '0' || ch > '9')
            return ShowPromptInfo(-2);
    }

	errNo= atoi(error.GetBuffer(0));
	char *presultInfo=psbcStampApi::Instance()->geterrMsg(errNo);

	CString tmpErrorInfo;
    tmpErrorInfo.AppendFormat("%s",presultInfo);
	ShowPromptInfo(0, tmpErrorInfo);
}

void CBasicFunDialog::OnBnClickedbtnclosecon()
{
	int result = psbcStampApi::Instance()->disconnMachine()==0?0:-1;
	ShowPromptInfo(result);
}

void CBasicFunDialog::OnBnClickedbtncheckcon()
{
    // 0-- - 成功, -1-- - 失败, 其它错误码
	int result = psbcStampApi::Instance()->isConnMachine();
    switch (result) {
    case 0:
        ShowPromptInfo(7);
        break;
    case -1:
        ShowPromptInfo(8);
        break;
    default:
        ShowPromptInfo(-1);
        break;
    }
}

void CBasicFunDialog::OnBnClickedbtnslotinfo()
{
	// TODO: 在此添加控件通知处理程序代码
    CString m_machinNumInfo;
    m_sealEditInfo.GetWindowText(m_machinNumInfo);
    if (m_machinNumInfo.IsEmpty()) {
        MessageBox("请输入查询设备编号");
        return;
    }

    char* machine_num = m_machinNumInfo.GetBuffer(m_machinNumInfo.GetLength());
    m_machinNumInfo.ReleaseBuffer();

    char *pinfoResult = psbcStampApi::Instance()->querySlotInfo(machine_num);
    CString temp;
    if (pinfoResult != NULL)
    {
        temp.AppendFormat("输出信息:%s", pinfoResult);
    }
    else
    {
        temp.AppendFormat("查询信息失败:%s", pinfoResult);
    }
	
	ShowPromptInfo(0,temp.GetBuffer(0));
}

BOOL CBasicFunDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    psbcStampApi::Instance()->connMachine("OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
