#include "stdafx.h"
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>
#include "task_mgr.h"
#include "common_definitions.h"

MC::TaskMgr* MC::TaskMgr::task_inst = NULL;

MC::TaskMgr* MC::TaskMgr::GetInst()
{
    if (NULL == task_inst)
        task_inst = new MC::TaskMgr;

    return task_inst;
}

std::string MC::TaskMgr::GeneTask()
{
    boost::lock_guard<boost::mutex> lk(task_mtx_);
    std::string task_id = GenerateGUID();
//    task_map_.insert(std::make_pair(task_id, MC::TS_ALIVE));
    task_map_.insert(std::pair(task_id, MC::TS_ALIVE));
    return task_id;
}

enum MC::TaskState MC::TaskMgr::QueryTaskState(const std::string& task_id)
{
    boost::lock_guard<boost::mutex> lk(task_mtx_);
    std::map<std::string, MC::TaskState>::const_iterator it = task_map_.begin();
    for (; it != task_map_.end(); ++it) {
        if (it->first == task_id)
            return it->second;
    }

    return MC::TS_NON_EXIST;
}

void MC::TaskMgr::MarkUsed(const std::string& task_id)
{
    boost::lock_guard<boost::mutex> lk(task_mtx_);
    std::map<std::string, MC::TaskState>::iterator it = task_map_.begin();
    for (; it != task_map_.end(); ++it) {
        if (task_id == it->first) {
            it->second = MC::TS_USED;
        }
    }
}

bool MC::TaskMgr::RemoveTask(const std::string& task_id)
{
    boost::lock_guard<boost::mutex> lk(task_mtx_);
    std::map<std::string, MC::TaskState>::iterator it = task_map_.begin();
    for (; it != task_map_.end(); ++it) {
        if (task_id == it->first) {
            it->second = MC::TS_DEAD;
            return true;
        }
    }

    return false;
}

std::string MC::GenerateGUID(int bits)
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string uuid_str = boost::lexical_cast<std::string>(uuid);
    return uuid_str;
}
