#include "stdafx.h"
#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/exception/all.hpp>
#include "parse.h"
#include "log.h"
#include "common_definitions.h"

MC::SvrConfig* MC::SvrConfig::config_inst = NULL;

bool MC::SvrConfig::Parse()
{
    std::string path;
    if (!MC::GetMoudulePath(path))
        return false;

    std::string xml_path = path + "server.xml";
    try {
        boost::property_tree::ptree pt;
        boost::property_tree::xml_parser::read_xml(xml_path, pt);
        // 章到位等待时间
        wait_time_ = atoi(pt.get<std::string>("con.wait_time").c_str());

        // 校准点
        std::string pt_str = pt.get<std::string>("con.checkpoint.point0");
        size_t comma_pos = pt_str.find_first_of(",");
        if (comma_pos != std::string::npos) {
            check_pt0_.x = atoi(pt_str.substr(0, comma_pos).c_str());
            check_pt0_.y = atoi(pt_str.substr(comma_pos + 1, std::string::npos).c_str());
        }

        pt_str = pt.get<std::string>("con.checkpoint.point1");
        comma_pos = pt_str.find_first_of(",");
        if (comma_pos != std::string::npos) {
            check_pt1_.x = atoi(pt_str.substr(0, comma_pos).c_str());
            check_pt1_.y = atoi(pt_str.substr(comma_pos + 1, std::string::npos).c_str());
        }

        pt_str = pt.get<std::string>("con.checkpoint.point2");
        comma_pos = pt_str.find_first_of(",");
        if (comma_pos != std::string::npos) {
            check_pt2_.x = atoi(pt_str.substr(0, comma_pos).c_str());
            check_pt2_.y = atoi(pt_str.substr(comma_pos + 1, std::string::npos).c_str());
        }

        pt_str = pt.get<std::string>("con.checkpoint.point3");
        comma_pos = pt_str.find_first_of(",");
        if (comma_pos != std::string::npos) {
            check_pt3_.x = atoi(pt_str.substr(0, comma_pos).c_str());
            check_pt3_.y = atoi(pt_str.substr(comma_pos + 1, std::string::npos).c_str());
        }

        pt_str = pt.get<std::string>("con.checkpoint.point4");
        comma_pos = pt_str.find_first_of(",");
        if (comma_pos != std::string::npos) {
            check_pt4_.x = atoi(pt_str.substr(0, comma_pos).c_str());
            check_pt4_.y = atoi(pt_str.substr(comma_pos + 1, std::string::npos).c_str());
        }

        // 凭证摄像头图片格式
        img_format_ = atoi(pt.get<std::string>("con.img_format").c_str());

        // 凭证摄像头分辨率
        resolution_width_ = atoi(pt.get<std::string>("con.resolution.width").c_str());
        resolution_height_ = atoi(pt.get<std::string>("con.resolution.height").c_str());

        // 凭证摄像头补光灯亮度
        brightness_ = atoi(pt.get<std::string>("con.brightness").c_str());
    } catch (...) {
        boost::exception_ptr e = boost::current_exception();
        Log::WriteLog(LL_ERROR, "Config::Parse->%s",
            boost::current_exception_diagnostic_information());
        return false;
    }

    return true;
}

