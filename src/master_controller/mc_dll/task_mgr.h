#ifndef MASTERCTRL_TASK_MGR_H_
#define MASTERCTRL_TASK_MGR_H_

#include <string>
#include <map>
#include <boost/thread/thread.hpp>
#include "common_definitions.h"

namespace MC {

// 管理用印任务号
class TaskMgr {

public:
    static TaskMgr* GetInst();

    // 生成唯一用印任务号
    std::string GeneTask();
   
    // 查询用印任务号状态
    enum TaskState QueryTaskState(const std::string& task_id);

    void MarkUsed(const std::string& task_id);

    // 删除用印任务号
    bool RemoveTask(const std::string& task_id);

private:
    TaskMgr() {

    }

private:
    boost::mutex                            task_mtx_;
    std::map<std::string, enum TaskState>   task_map_;

    static TaskMgr* task_inst;
};  // TaskMgr

std::string GenerateGUID(int bits = 16);

} // namespace MC

#endif // MASTERCTRL_TASK_MGR_H_
