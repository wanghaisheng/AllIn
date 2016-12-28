#ifndef CONTROLLER_BOC_API_H_
#define CONTROLLER_BOC_API_H_

#include <string>
#include <iostream>
#include <boost/timer.hpp>
#include <boost/asio.hpp>
#include "common_definitions.h"
#include "base_api.h"

#pragma warning(disable:4251)

namespace MC {

// 统一异步通知对象
class MASTERCTRL_API NotifyResult {
public:
    virtual void Notify(
        ErrorCode ec,               // 具体处理结果
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "") = 0;

    virtual ~NotifyResult() {}
};

// 中行API
class MASTERCTRL_API BOCApi: public BaseAPI {
public:
    static BOCApi* GetInst() {
        if (NULL == inst_) {
            inst_ = new (std::nothrow) MC::BOCApi("中行");
        }

        return inst_;
    }

private:
    BOCApi(std::string des) : BaseAPI(des) {
        int out_x;
        int out_y;
        double out_angle;
        int ret = STSeachSealPoint(
            "K:\\pj\\src\\master_controller\\bin\\ori.jpg",
            100,
            100,
            0,
            out_x,
            out_y,
            out_angle);

        std::string        out_model_type;
        std::string        voucher_no;
        std::string       trace_no;
        int               x;
        int               y;
        int               angle;
        ret = IdentifyImage(
            "K:\\pj\\src\\master_controller\\bin\\cut.jpg",
            "华东三省一市",
            out_model_type,
            voucher_no,
            trace_no,
            x,
            y,
            angle);
    }

private:
    static MC::BOCApi* inst_;  // 单例

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
    void OperateBeep(int operation, NotifyResult* notify);

    // 卡槽数量查询
    void QuerySlot(NotifyResult* notify);

    // 报警器操作
    void OperateAlarm(int alarm, int ctrl, NotifyResult* notify);

    // 查询已绑定MAC地址
    void QueryMAC(NotifyResult* notify);

private:
    std::string des_;
};

}

#endif // CONTROLLER_STAMPER_API_H_
