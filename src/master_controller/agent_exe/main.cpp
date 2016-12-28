#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "agent_cmd.h"
#include "api.h"

int main(int argc, TCHAR *argv[])
{
    // 获取印控仪编号
    std::string sn;
    int ret = QueryMachine(sn);
    printf("main->获取印控仪编号: %d, 编号:%s\n", ret, sn.c_str());

    // 初始化
    ret = InitMachine("51534sDf");
    printf("main->初始化印控机: %d\n", ret);

    // 绑定MAC
    int bind_times = 1;
    while (bind_times--) {
        ret = BindMAC("FFDE03AB");
        printf("main->绑定MAC地址: %d\n", ret);
    }

    // 解绑MAC
    ret = UnbindMAC("30-3A-64-D6-FD-30");
    printf("main->解绑MAC地址: %d\n", ret);

    // 准备用印
    std::string task_id;
//     ret = PrepareStamp(1, 20, task_id);
//     printf("main->准备用印: %d, 任务号: %s\n",
//         ret,
//         task_id.c_str());

    // 拍照
    std::string ori = "G:\\pj\\src\\master_controller\\bin\\ori.jpg";
    std::string cut = "G:\\pj\\src\\master_controller\\bin\\cut.jpg";
    ret = Snapshot(200, 109, ori, cut);
    printf("main->拍照: %d\n", ret);

    // 照片合成
    std::string p1 = "G:\\pj\\src\\master_controller\\bin\\ori.jpg";
    std::string p2 = "G:\\pj\\src\\master_controller\\bin\\cut.jpg";
    std::string merged = "G:\\pj\\src\\master_controller\\bin\\merged.jpg";
    ret = MergePhoto(p1, p2, merged);
    printf("main->合成照片: %d\n", ret);

    // 验证码识别
    std::string template_id;
    std::string trace_num;
    ret = RecognizeImage(p1, template_id, trace_num);
    printf("main->验证码识别: %d\n", ret);

    // 要素识别
    std::string result;
    ret = IdentifyElement(p1,
        100,
        220,
        40,
        30,
        90, 
        result);
    printf("main->要素识别: %d\n", ret);

    // 普通用印
    ret = OrdinaryStamp("OrdinaryStamp001", "本票", 2, 200, 100, 180);
    printf("main->普通用印: %d\n", ret);

    // 自动用印
    ret = AutoStamp("AutoStamp001", "汇票", 1);
    printf("main->自动用印: %d\n", ret);

    // 用印结束
    ret = FinishStamp("FinishStamp001");
    printf("main->用印结束: %d\n", ret);

    // 释放印控机
    ret = ReleaseStamp("STDZ_RELEASE_STAMPER_20161019_001");
    printf("main->释放印控机: %d\n", ret);

    // 获取错误信息
    std::string err_msg;
    std::string err_resolver;
    ret = GetError(10, err_msg, err_resolver);
    printf("main->获取错误信息: %d, 错误描述: %s, 解决方案: %s\n", 
        ret, 
        err_msg.c_str(), 
        err_resolver.c_str());

    // 校准印章
    ret = Calibrate(1);
    printf("main->校准印章: %d\n", ret);

    // 印章状态查询
    int stamper_status[7] = { 0 };
    ret = QueryStampers(stamper_status);
    printf("main->印章状态查询: %d\n", ret);

    // 安全门状态查询
    int safe_status;
    ret = QuerySafe(safe_status);
    printf("main->安全门状态查询: %d\n", ret);

    // 开关安全门
    ret = ControlSafe(1);
    printf("main->开关安全门: %d\n", ret);

    // 控制蜂鸣器
    ret = ControlBeep(0);
    printf("main->控制蜂鸣器: %d\n", ret);

    // 获取卡槽数
    int slot_num = 0;
    ret = QuerySlot(slot_num);
    printf("main->获取卡槽数: %d, 卡槽数: %d\n", ret, slot_num);

    getchar();
    return ret;
}
