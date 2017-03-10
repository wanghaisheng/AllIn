#include "files.h"

Files* Files::files_inst_ = NULL;

Files* Files::getInst()
{
    if (NULL == files_inst_)
        files_inst_ = new Files;

    return files_inst_;
}

bool Files::load(const std::string& path)
{

}

bool Files::getFileList(const std::string& key, std::vector<std::string>& file_list) const
{

}
