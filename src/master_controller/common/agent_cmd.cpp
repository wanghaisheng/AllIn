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
    xs_ << original_dpi_ << cut_dpi_ << original_path_ << cut_path_ << ret_;
    xs_.Trim();
}

void SnapshotCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> original_dpi_ >> cut_dpi_ >> original_path_ >> cut_path_ >> ret_;
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
    xs_ << task_id_ << type_ << stamper_num_ << x_ << y_ << angle_ << ret_;
    xs_.Trim();
}

void OridinaryStampCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> task_id_ >> type_ >> stamper_num_ >> x_ >> y_ >> angle_ >> ret_;
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
    for (int i = 0; i < MAX_STAMPER_NUM; ++i)
        xs_ << stamper_status_[i];
    xs_ << ret_;
    xs_.Trim();
}

void QueryStampersCmd::Unser()
{
    BaseCmd::Unser();
    for (int i = 0; i < MAX_STAMPER_NUM; ++i)
        xs_ >> stamper_status_[i];
    xs_ >> ret_;
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
    xs_ << ctrl_ << ret_;
    xs_.Trim();
}

void BeepCtrlCmd::Unser()
{
    BaseCmd::Unser();
    xs_ >> ctrl_ >> ret_;
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
