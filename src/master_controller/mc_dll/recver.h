#ifndef CONTROLLER_RECVER_H_
#define CONTROLLER_RECVER_H_

//#define WIN32_LEAN_AND_MEAN

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/thread/thread.hpp>
#include <windows.h>
#include "syn_queue.h"
#include "pipe_server.h"
#include "mq_cnn.h"

#ifdef _WIN32
#ifdef MASTERCTRL_EXPORTS
#define MASTERCTRL_API _declspec(dllexport)
#else
#define MASTERCTRL_API _declspec(dllimport)
#endif
#else
#define MASTERCTRL_API
#endif

class MASTERCTRL_API Recver: public MQCnn {
public:
    bool Start();

    static void Insert(const RecvMsg* msg);

    static void Signal();

    void Stop();

    bool WriteResp(LPPIPEINST pipe_inst_, char* buf, unsigned int priority = 0);

protected:
    virtual void OnRecvMQMsg(char* buf, int size);

private:
    bool ParseConfig();

    bool StartMQ();

    bool StartPipe();

    void HandleQueryMachine(const RecvMsg* msg);

    void HandleSetMachine(const RecvMsg* msg);

    void HandleInitMachine(const RecvMsg* msg);

    void HandleBindMAC(const RecvMsg* msg);

    void HandleUnbindMAC(const RecvMsg* msg);

    void HandlePrepareStamp(const RecvMsg* msg);

    void HandleQueryPaper(const RecvMsg* msg);

    void HandleSnapshot(const RecvMsg* msg);

    void HandleMergePhoto(const RecvMsg* msg);

    void HandleSearchStampPoint(const RecvMsg* msg);
    
    void HandleRecognition(const RecvMsg* msg);

    void HandleElementIdenti(const RecvMsg* msg);

    void HandleRegcoEtc(const RecvMsg* msg);

    void HandleOrdinary(const RecvMsg* msg);

    void HandleAuto(const RecvMsg* msg);

    void HandleFinish(const RecvMsg* msg);

    void HandleReleaseStamper(const RecvMsg* msg);

    void HandleGetError(const RecvMsg* msg);

    void HandleCalibrate(const RecvMsg* msg);

    void HandleQueryStampers(const RecvMsg* msg);

    void HandleQuerySafe(const RecvMsg* msg);

    void HandleSafeControl(const RecvMsg* msg);

    void HandleBeepControl(const RecvMsg* msg);

    void HandleQuerySlot(const RecvMsg* msg);

    void HandleAlarmControl(const RecvMsg* msg);

    void HandleQueryAlarm(const RecvMsg* msg);

    void HandleQueryMAC(const RecvMsg* msg);

    void HandleLock(const RecvMsg* msg);
    
    void HandleUnlock(const RecvMsg* msg);

    void HandleQueryLock(const RecvMsg* msg);

    void HandleOpenCnn(const RecvMsg* msg);

    void HandleCloseCnn(const RecvMsg* msg);

    void HandleQueryCnn(const RecvMsg* msg);

    void HandleSetSideDoor(const RecvMsg* msg);

    void HandleGetDevModel(const RecvMsg* msg);

    void HandleOpenPaper(const RecvMsg* msg);

    void HandleCtrlLED(const RecvMsg* msg);

    void HandleCheckParam(const RecvMsg* msg);

    void HandleOpenCamera(const RecvMsg* msg);

    void HandleCloseCamera(const RecvMsg* msg);

    void HandleGetCameraStatus(const RecvMsg* msg);

    void HandleSetResolution(const RecvMsg* msg);

    void HandleSetDPI(const RecvMsg* msg);

    void HandleSetProperty(const RecvMsg* msg);

    void HandleRecordVideo(const RecvMsg* msg);

    void HandleStopRecordVideo(const RecvMsg* msg);

    void HandleGetRFID(const RecvMsg* msg);

    void HandleGetDevStatus(const RecvMsg* msg);

    void HandleCvtCoord(const RecvMsg* msg);

    void HandleWriteRatio(const RecvMsg* msg);

    void HandleReadRatio(const RecvMsg* msg);

    void HandleWriteCali(const RecvMsg* msg);

    void HandleReadCali(const RecvMsg* msg);

    void HandleHeart(const RecvMsg* msg);

    void HandleQueryTop(const RecvMsg* msg);

    void HandleEnterMain(const RecvMsg* msg);

    void HandleExitMain(const RecvMsg* msg);

    void HandleStartPreview(const RecvMsg* msg);

    void HandleStopPreview(const RecvMsg* msg);

    void HandleFactoryCtrl(const RecvMsg* msg);

    void HandleReset(const RecvMsg* msg);

    void HandleRestart(const RecvMsg* msg);

    void HandleGetSystem(const RecvMsg* msg);

    void HandleReadMainSpare(const RecvMsg* msg);

    void HandleWriteMainSpare(const RecvMsg* msg);

    void HandleRecogQR(const RecvMsg* msg);

    void HandleCalcRatio(const RecvMsg* msg);

    void HandleFind2Circles(const RecvMsg* msg);

    void HandleFind4Circles(const RecvMsg* msg);

    void HandleSetStamp(const RecvMsg* msg);

private:
    static MC::SynQueue<const RecvMsg*> recver_queue_;
    static HANDLE                       recv_msg_ev_;

    boost::mutex                        write_resp_mtx_;

    MC::ConnType                        cnn_type_;
};

#endif // CONTROLLER_RECVER_H_
