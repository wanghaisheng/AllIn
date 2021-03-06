﻿//
// Created by kailani on 2016/12/30.
//

#ifndef MC_DLL_IMG_PRO_H
#define MC_DLL_IMG_PRO_H

#include <string>
#include <windows.h>
#include "parse.h"

enum FontLib {
    image = 0,
    eng = 1
};

namespace MC {
    class ImgPro {
    public:
        static ImgPro* GetInst() {
            if (g_inst_ == NULL)
                g_inst_ = new ImgPro();

            return g_inst_;
        }

        ~ImgPro() {
        }

        // 根据切图用印点像素坐标获取盖章点基于切图中心点位置, 像素坐标
        void CaculatePos(
                int pCenterX,   // 原图宽/2
                int pCenterY,   // 原图高/2
                int inX,        // 切图用印点像素x坐标
                int inY,        // 切图用印点像素y坐标
                int& outX,
                int& outY,
                int& angle);

        // 根据切图中心点坐标获取原图像素坐标和用印角度
        int STSeachSealPoint(
                const char* in_src_img_name,
                int in_x,
                int in_y,
                double in_angle,
                int &out_x,
                int &out_y,
                double &out_angle);

        // 将原图用印像素坐标转换为设备(印控机)用印坐标(毫米)
        MC::Point* GetSealCoord(int nX, int nY);

        // 图片校正并切割黑边
        int CutImage(
                const std::string&  ori,
                std::string&        cut);

        // 合并图片
        int MergeImage(
                const std::string& img1,
                const std::string& img2,
                const std::string& merged);

        // 模板类型、角度、用印点识别
        // 输入参数:
        // @img_path    --- 原图片路径
        // 输出参数:
        // @model_type  --- 模板类型
        // @outangle    --- 旋转角度
        // @x           --- 用印点的 x 坐标(模板上的)
        // @y           --- 用印点的 y 坐标(模板上的)
        int GetModelTypeAnglePoint(
                const std::string&  img_path,       // 原图片路径
                char*               model_type,     // 模板类型
                double&             outangle,       // 旋转角度
                int&                x,
                int&                y);

        // 版面、验证码识别
        // 输入参数:
        // @cut_img     --- 裁剪后的图片路径
        // @model_type  --- 输入的模板类型
        // 输出参数:
        // @out_model_type  --- 识别出的模板类型
        // @voucher_no      --- 凭证编号
        // @trace_no        --- 追溯码
        // @x               --- 相对于裁剪后图片的用印x坐标
        // @y               --- 相对于裁剪后图片的用印y坐标
        // @angle           --- 旋转角度
        int IdentifyImage(
            const        std::string&  cut_img,
            char*        model_type,
            char*        out_model_type,
            char*        voucher_no,
            char*        trace_no,
            int&         x,
            int&         y,
            int&         angle);

        // 要素区域识别
        // img      --- 原图路径
        // x        --- 左上角x坐标
        // y        --- 左上角y坐标
        // width    --- 宽
        // height   --- 高
        // angle    --- 旋转角度
        // result   --- 识别结果
        int IdentifyArea(
                const std::string&  img,
                int                 x,
                int                 y,
                int                 width,
                int                 height,
                char*               result);

        int SearchSrcImgStampPoint(
            const std::string& img,
            int in_x,
            int in_y,
            double in_angle,
            int& out_x,
            int& out_y,
            double& out_angle);

        int ReadCodebar(
            const std::string& filename,
            char* text);

        int ReadCodebarByRect(
            const std::string& filename,
            int left,
            int top,
            int right,
            int bottom,
            char* text);

        int CalculateRatio(
            const char* src_file_name, 
            int dpi, 
            double &rate_x, 
            double &rate_y);

        int Find2Circles2(
            const std::string& file,
            char* buf);

        int Find4Circles2(
            const std::string& file,
            char* buf);

    private:
        static ImgPro* g_inst_;

        HINSTANCE dllinst_;
    };
}

#endif //MC_DLL_IMG_PRO_H
