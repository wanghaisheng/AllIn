#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include "parse.h"
#include "common_definitions.h"

Config* Config::config_inst = NULL;

bool Config::Parse()
{
    std::string path;
    if (!MC::GetMoudulePath(path))
        return false;

    std::string xml_path = path + "api_config.xml";
    try {
        boost::property_tree::ptree pt;
        boost::property_tree::xml_parser::read_xml(xml_path, pt);
        ori_path_ = pt.get<std::string>("con.ori");
        cut_path_ = pt.get<std::string>("con.cut");
    }
    catch (std::exception& e) {
        std::cout << "Config::Parse->" << e.what() << std::endl;
        return false;
    }

    return true;
}

