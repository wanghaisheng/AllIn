#ifndef CONTROLLER_BOC_API_H_
#define CONTROLLER_BOC_API_H_

#include <string>
#include <iostream>
#include <boost/timer.hpp>
#include <boost/asio.hpp>
#include "common_definitions.h"
#include "img_pro.h"
#include "base_api.h"

#pragma warning(disable:4251)

namespace MC {

// 统一异步通知对象
class NotifyResult {
public:
    virtual void Notify(
        ErrorCode ec,               // 具体处理结果
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "") = 0;

    virtual ~NotifyResult() {}
};

// supported api by master controller now.
class STSealAPI: public BaseAPI {
public:
    static STSealAPI* GetInst() {
        if (NULL == inst_) {
            inst_ = new (std::nothrow) MC::STSealAPI("三泰");
        }

        return inst_;
    }

private:
    STSealAPI(std::string des) : BaseAPI(des) {
    }

private:
    static MC::STSealAPI* inst_;  // 单例

public:
    // 获取用印机编号
    void QueryMachine(NotifyResult* notify);

    // 设置印控机编号
    void SetMachine(const std::string& sn, NotifyResult* notify);

    // 初始化用印机
    // key   --- 用印机认证码
    void InitMachine(std::string key, NotifyResult* notify);

    // 绑定MAC地址
    // mac   --- 待绑定MAC地址
    void BindMAC(std::string mac, NotifyResult* notify);

    // 解绑MAC地址
    // mac   --- 待解绑MAC地址, 该MAC地址应在已绑定MAC地址列表中
    void UnbindMAC(std::string mac, NotifyResult* notify);

    // 准备用印
    // stamp_num    --- 印章号(1-6)
    // timeout      --- 进纸门超时未关闭(秒)
    void PrepareStamp(int stamp_num, int timeout, NotifyResult* notify);

    // 查询进纸门状态
    void QueryPaperDoor(NotifyResult* notify);

    // 拍照
    // ori_dpi      --- 原始图像DPI
    // cut_dpi      --- 剪切后图像DPI
    void Snapshot(
        int which,
        int ori_dpi, 
        int cut_dpi, 
        const std::string& ori_path, 
        const std::string& cut_path,
        NotifyResult* notify);

    // 合成照片
    void MergePhoto(
        const std::string& photo1, 
        const std::string& photo2,
        const std::string& merged,
        NotifyResult* notify);

    void SearchSrcImageStampPoint(
        const std::string&  src_img_name,
        int                 in_x,
        int                 in_y,
        double              in_angle,
        NotifyResult*       notify);

    // 版面、验证码识别
    void RecognizeImage(
        const std::string& img,
        NotifyResult* notify);

    // 要素识别
    void IdentifyElement(
        const std::string& path,
        int x,
        int y,
        int width,
        int height,
        int angle_,
        NotifyResult* notify);

    // 普通用印
    void OrdinaryStamp(
        const std::string& task,
        const std::string& voucher, 
        int num, 
        int ink,
        int x, 
        int y, 
        int angle, 
        int type,
        NotifyResult* notify);

    // 自动用印
    void AutoStamp(
        const std::string& task,
        const std::string& voucher, 
        int num, 
        NotifyResult* notify);

    // 结束用印
    void FinishStamp(
        const std::string& task, 
        NotifyResult* notify);

    // 释放印控机
    // 解锁印控仪, 可以在任意时刻调用
    void ReleaseStamp(
        const std::string& machine, 
        NotifyResult* notify);

    // 获取错误信息
    void GetError(
        int err_code, 
        NotifyResult* notify);

    // 校准印章
    // num      --- 印章卡槽号, 从1开始
    void CalibrateMachine(
        int num, 
        NotifyResult* notify);

    // 印章状态查询
    void QueryStampers(NotifyResult* notify);

    // 安全门状态查询
    void QuerySafeDoor(NotifyResult* notify);

    // 开关安全门(电子锁)
    // operation        --- 0--关, 1--开
    // timeout          --- 超时仍未关闭安全门, 需鸣蜂鸣器, 单位:秒
    void OperateSafeDoor(int operation, int timeout, NotifyResult* notify);

    // 开关蜂鸣器
    // operation        --- 0--关, 1--开
    void OperateBeep(int operation, int type, int interval, NotifyResult* notify);

    // 卡槽数量查询
    void QuerySlot(NotifyResult* notify);

    // 报警器操作
    void OperateAlarm(int alarm, int ctrl, NotifyResult* notify);

    // 查询已绑定MAC地址
    void QueryMAC(NotifyResult* notify);

    // lock machine
    void Lock(NotifyResult* notify);

    // unlock machine
    void Unlock(NotifyResult* notify);

    // query lock status
    void QueryLock(NotifyResult* notify);

    // open connection
    void Connect(NotifyResult* notify);

    void Disconnect(NotifyResult* notify);

    void QueryCnn(NotifyResult* notify);

    void SetSideDoor(int keep_open, int timeout, NotifyResult* notify);

    void QueryDevModel(NotifyResult* notify);

    void OpenPaper(int timeout, NotifyResult* notify);

    // 补光灯控制
    void CtrlLed(int which, int switchs, int value, NotifyResult* notify);

    // 用印参数合法性检查
    void CheckParams(int x, int y, int angle, NotifyResult* notify);

    void OpenCamera(int which, NotifyResult* notify);

    void CloseCamera(int which, NotifyResult* notify);

    void QueryCameraStatus(int which, NotifyResult* notify);

    void SetResolution(int which, int x, int y, NotifyResult* notify);

    void SetDPI(int which, int x, int y, NotifyResult* notify);

    void SetProperty(int which, NotifyResult* notify);

    void RecordVideo(int which, const std::string& path, NotifyResult* notify);

    void StopRecordVideo(int which, const std::string& path, NotifyResult* notify);

    void GetRFID(int slot, NotifyResult* notify);

}; // end class STSealAPI

} // end namespace MC

#endif // CONTROLLER_STAMPER_API_H_
