#include "stdafx.h"
#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/exception/all.hpp>
#include "parse.h"
#include "SealLog.h"

#ifndef PATHSPLIT_CHAR
#define PATHSPLIT_CHAR      '\\'
#endif

PSBCConfig* PSBCConfig::config_inst = NULL;

bool GetMoudulePath(std::string& path)
{
    char file_path[_MAX_PATH] = { 0 };
    if (GetModuleFileName(NULL, file_path, _MAX_FNAME) == 0)
        return false;

    std::string file_path_str = file_path;
    size_t last_slash = file_path_str.find_last_of(PATHSPLIT_CHAR);
    if (last_slash == std::string::npos)
        return false;

    path = file_path_str.substr(0, last_slash + 1);
    return true;
}

bool PSBCConfig::Parse()
{
    std::string path;
    if (!GetMoudulePath(path))
        return false;

    std::string xml_path = path + "configure.xml";
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

        // 切图DPI值
        dpi_ = atoi(pt.get<std::string>("con.dpi").c_str());

        // 凭证摄像头分辨率
        resolution_width_ = atoi(pt.get<std::string>("con.resolution.width").c_str());
        resolution_height_ = atoi(pt.get<std::string>("con.resolution.height").c_str());

        // 凭证摄像头补光灯亮度
        brightness_ = atoi(pt.get<std::string>("con.brightness").c_str());
    } catch (...) {
        boost::exception_ptr e = boost::current_exception();
        return false;
    }

    return true;
}

