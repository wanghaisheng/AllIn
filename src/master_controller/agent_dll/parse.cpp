#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/exception/all.hpp>
#include "parse.h"
#include "log.h"

MC::Config* MC::Config::config_inst = NULL;

bool MC::Config::Parse()
{
    std::string path;
    if (!MC::GetMoudulePath(path))
        return false;

    std::string type;
    std::string name;
    std::string xml_path = path + "mc.xml";
    try {
        boost::property_tree::ptree pt;
        boost::property_tree::xml_parser::read_xml(xml_path, pt);
        type = pt.get<std::string>("con.type");
        name = pt.get<std::string>("con.name");
    } catch (...) {
        boost::exception_ptr e = boost::current_exception();
        Log::WriteLog(LL_ERROR, "MC::Config::Parse->%s",
            boost::current_exception_diagnostic_information());
        return false;
    }

    if (0 == strcmp(type.c_str(), "PIPE")) {            // 管道
        conn_type_ = MC::CT_PIPE;

        char cnn_name[1024] = { 0 };
        sprintf_s(cnn_name, "\\\\.\\pipe\\%s", name.c_str());
        pipe_name_ = cnn_name;
    } else if (0 == strcmp(type.c_str(), "MQ")) {       // 消息队列
        conn_type_ = MC::CT_MQ;

        send_mq_name_ = name;
        recv_mq_name_ = name + "copy";
    } else {                                          // 不支持的通信方式, 退出
        Log::WriteLog(LL_ERROR, "MC::Config::Parse->不支持的通信方式, 直接退出");
        return false;
    }

    return true;
}
