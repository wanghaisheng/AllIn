#include "agent_cmd.h"

void BaseCmd::Ser()
{
    xs_ << ct_ << send_time_;
}

void BaseCmd::Unser()
{
    xs_.Untrim();
    xs_ >> ct_ >> send_time_;
}

void InitMachineCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << key_ << ret_;
    xs_.Trim();
}

void InitMachineCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> key_ >> ret_;
}

void BindMACCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << mac_ << ret_;
    xs_.Trim();
}

void BindMACCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> mac_ >> ret_;
}

void UnbindCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << mac_ << ret_;
    xs_.Trim();
}

void UnbindCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> mac_ >> ret_;
}

void PrepareStampCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << stamper_id_ << timeout_ << task_id_ << ret_;
    xs_.Trim();
}

void PrepareStampCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> stamper_id_ >> timeout_ >> task_id_ >> ret_;
}

void ViewPaperCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << status_ << ret_;
    xs_.Trim();
}

void ViewPaperCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> status_ >> ret_;
}

void SnapshotCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << original_dpi_ << cut_dpi_ << original_path_ << cut_path_ << ret_;
    xs_.Trim();
}

void SnapshotCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> original_dpi_ >> cut_dpi_ >> original_path_ >> cut_path_ >> ret_;
}

void SynthesizePhotoCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << photo1_ << photo2_ << merged_ << ret_;
    xs_.Trim();
}

void SynthesizePhotoCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> photo1_ >> photo2_ >> merged_ >> ret_;
}

void RecognitionCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << path_ << template_id_ << trace_num_ << ret_;
    xs_.Trim();
}

void RecognitionCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> path_ >> template_id_ >> trace_num_ >> ret_;
}

void RecoModelTypeEtcCmd::Ser() {
    BaseCmd::Ser();
    xs_ << path_ << model_result_ << angle_ << x_ << y_ << ret_;
    xs_.Trim();
}

void RecoModelTypeEtcCmd::Unser() {
    BaseCmd::Unser();
    xs_ >> path_ >> model_result_ >> angle_ >> x_ >> y_ >> ret_;
}

void SearchStampPointCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << src_ << in_x_ << in_y_ << in_angle_ << out_x_ << out_y_ << out_angle_;
    xs_.Trim();
}

void SearchStampPointCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> src_ >> in_x_ >> in_y_ >> in_angle_ >> out_x_ >> out_y_ >> out_angle_;
}

void IdentifyElementCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << path_ << x_ << y_ << width_ << height_ << angle_ << content_str_ << ret_;
    xs_.Trim();
}

void IdentifyElementCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> path_ >> x_ >> y_ >> width_ >> height_ >> angle_ >> content_str_ >> ret_;
}

void OridinaryStampCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << task_id_ << type_ << stamper_num_ << ink_ << x_ << y_ << angle_
        << seal_type_ << ret_;
    xs_.Trim();
}

void OridinaryStampCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> task_id_ >> type_ >> stamper_num_ >> ink_ >> x_ >> y_ >> angle_ 
        >> seal_type_ >> ret_;
}

void AutoStampCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << task_id_ << type_ << stamper_num_ << ret_;
    xs_.Trim();
}

void AutoStampCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> task_id_ >> type_ >> stamper_num_ >> ret_;
}

void FinishStampCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << task_id_ << ret_;
    xs_.Trim();
}

void FinishStampCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> task_id_ >> ret_;
}

void ReleaseStamperCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << stamp_id_ << ret_;
    xs_.Trim();
}

void ReleaseStamperCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> stamp_id_ >> ret_;
}

void GetErrorCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << err_ << err_msg_ << err_resolver_ << ret_;
    xs_.Trim();
}

void GetErrorCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> err_ >> err_msg_ >> err_resolver_ >> ret_;
}

void HeartCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << ret_;
    xs_.Trim();
}

void HeartCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> ret_;
}

void QueryMachineCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << sn_ << ret_;
    xs_.Trim();
}

void QueryMachineCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> sn_ >> ret_;
}

void SetMachineCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << sn_ << ret_;
    xs_.Trim();
}

void SetMachineCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> sn_ >> ret_;
}

void QuerySlotCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << num_ << ret_;
    xs_.Trim();
}

void QuerySlotCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> num_ >> ret_;
}

void CalibrateCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << slot_ << ret_;
    xs_.Trim();
}

void CalibrateCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> slot_ >> ret_;
}

void QueryStampersCmd::Ser()
{
    BaseCmd::Ser();
//     for (int i = 0; i < MAX_STAMPER_NUM; ++i)
//         xs_ << stamper_status_[i];
    xs_ << stamper_status_ << ret_;
    xs_.Trim();
}

void QueryStampersCmd::Unser()
{
    BaseCmd::Unser();
//     for (int i = 0; i < MAX_STAMPER_NUM; ++i)
//         xs_ >> stamper_status_[i];
    xs_ >> stamper_status_ >> ret_;
}

void QuerySafeCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << status_ << ret_;
    xs_.Trim();
}

void QuerySafeCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> status_ >> ret_;
}

void SafeCtrlCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << ctrl_ << timeout_ << ret_;
    xs_.Trim();
}

void SafeCtrlCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> ctrl_ >> timeout_ >> ret_;
}

void BeepCtrlCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << ctrl_ << type_ << interval_ << ret_;
    xs_.Trim();
}

void BeepCtrlCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> ctrl_ >> type_ >> interval_ >> ret_;
}

void AlarmCtrlCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << alarm_ << ctrl_ << ret_;
    xs_.Trim();
}

void AlarmCtrlCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> alarm_ >> ctrl_ >> ret_;
}

void QueryAlarmCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << door_ << vibration_ << ret_;
    xs_.Trim();
}

void QueryAlarmCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> door_ >> vibration_ >> ret_;
}

void QueryMACCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << mac1_ << mac2_ << ret_;
    xs_.Trim();
}

void QueryMACCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> mac1_ >> mac2_ >> ret_;
}

void LockCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << ret_;
    xs_.Trim();
}

void LockCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> ret_;
}

void UnlockCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << ret_;
    xs_.Trim();
}

void UnlockCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> ret_;
}

void QueryLockCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << status_ << ret_;
    xs_.Trim();
}

void QueryLockCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> status_ >> ret_;
}

void OpenCnnCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << ret_;
    xs_.Trim();
}

void OpenCnnCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> ret_;
}

void CloseCnnCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << ret_;
    xs_.Trim();
}

void CloseCnnCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> ret_;
}

void QueryCnnCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << status_ << ret_;
    xs_.Trim();
}

void QueryCnnCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> status_ >> ret_;
}

void SideDoorAlarmCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << keep_open_ << timeout_ << ret_;
    xs_.Trim();
}

void SideDoorAlarmCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> keep_open_ >> timeout_ >> ret_;
}

void GetDevModelCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << model_ << ret_;
    xs_.Trim();
}

void GetDevModelCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> model_ >> ret_;
}

void OpenPaperCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << timeout_ << ret_;
    xs_.Trim();
}

void OpenPaperCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> timeout_ >> ret_;
}

void CtrlLEDCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << switch_ << value_ << ret_;
    xs_.Trim();
}

void CtrlLEDCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> switch_ >> value_ >> ret_;
}

void CheckParamCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << x_ << y_ << angle_ << ret_;
    xs_.Trim();
}

void CheckParamCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> x_ >> y_ >> angle_ >> ret_;
}

void OpenCameraCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << ret_;
    xs_.Trim();
}

void OpenCameraCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> ret_;
}

void CloseCameraCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << ret_;
    xs_.Trim();
}

void CloseCameraCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> ret_;
}

void QueryCameraStatusCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << status_ << ret_;
    xs_.Trim();
}

void QueryCameraStatusCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> status_ >> ret_;
}

void SetResolutionCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << x_ << y_ << ret_;
    xs_.Trim();
}

void SetResolutionCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> x_ >> y_ >> ret_;
}

void SetDPICmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << dpi_x_ << dpi_y_ << ret_;
    xs_.Trim();
}

void SetDPICmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> dpi_x_ >> dpi_y_ >> ret_;
}

void SetPropertyCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << hue_ << saturation_ << value_ << ret_;
    xs_.Trim();
}

void SetPropertyCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> hue_ >> saturation_ >> value_ >> ret_;
}

void RecordVideoCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << path_ << ret_;
    xs_.Trim();
}

void RecordVideoCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> path_ >> ret_;
}

void StopRecordVideoCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << which_ << path_ << ret_;
    xs_.Trim();
}

void StopRecordVideoCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> which_ >> path_ >> ret_;
}

void GetRFIDCmd::Ser() {
    BaseCmd::Ser();
    xs_ << slot_ << rfid_ << ret_;
    xs_.Trim();
}

void GetRFIDCmd::Unser() {
    BaseCmd::Unser();
    xs_ >> slot_ >> rfid_ >> ret_;
}

void GetDevStatusCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << status_code_ << ret_;
    xs_.Trim();
}

void GetDevStatusCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> status_code_ >> ret_;
}

void CoordCvtCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << x_img_ << y_img_ << x_dev_ << y_dev_ << ret_;
    xs_.Trim();
}

void CoordCvtCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> x_img_ >> y_img_ >> x_dev_ >> y_dev_ >> ret_;
}

void WriteRatioCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << x_ << y_ << ret_;
    xs_.Trim();
}

void WriteRatioCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> x_ >> y_ >> ret_;
}

void ReadRatioCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << x_ << y_ << ret_;
    xs_.Trim();
}

void ReadRatioCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> x_ >> y_ >> ret_;
}

void WriteCaliPtsCmd::Ser()
{
    BaseCmd::Ser();
    int i = 0;
    while (i < len_)
        xs_ << pts_[i++];
    xs_ << ret_;
    xs_.Trim();
}

void WriteCaliPtsCmd::Unser()
{
    BaseCmd::Unser();
    int i = 0;
    while (i < len_)
        xs_ >> pts_[i++];
    xs_ >> ret_;
}

void ReadCaliPtsCmd::Ser()
{
    BaseCmd::Ser();
    int i = 0;
    while (i < len_)
        xs_ << pts_[i++];
    xs_ << ret_;
    xs_.Trim();
}

void ReadCaliPtsCmd::Unser()
{
    BaseCmd::Unser();
    int i = 0;
    while (i < len_)
        xs_ >> pts_[i++];
    xs_ >> ret_;
}

void QueryTopCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << status_ << ret_;
    xs_.Trim();
}

void QueryTopCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> status_ >> ret_;
}

void ExitMaintainCmd::Ser()
{
    BaseCmd::Ser();
    xs_ << ret_;
    xs_.Trim();
}

void ExitMaintainCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> ret_;
}
