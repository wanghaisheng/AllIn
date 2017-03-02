//
// Created by kailani on 2016/12/30.
//

#include "stdafx.h"
#include "log.h"
#include "img_pro.h"
#include "ImgProcAndReco.h"

MC::ImgPro* MC::ImgPro::g_inst_ = NULL;

namespace MC {

    // 根据切图用印点像素坐标获取盖章点基于切图中心点位置, 像素坐标
    void ImgPro::CaculatePos(
            int pCenterX,   // 原图宽/2
            int pCenterY,   // 原图高/2
            int inX,        // 切图用印点像素x坐标
            int inY,        // 切图用印点像素y坐标
            int& outX,
            int& outY,
            int& angle)
    {
        angle %= 360;
        angle = angle < 0 ? 360 + angle : angle;
        switch (angle) {
            case 0:
            case 360:
                outX = inX - pCenterX;
                outY = inY - pCenterY;
                break;
            case 90:
                outX = inY - pCenterY;
                outY = pCenterX - inX;
                break;
            case 180:
                outX = pCenterX - inX;
                outY = pCenterY - inY;
                break;
            case 270:
                outX = pCenterY - inY;
                outY = inX - pCenterX;
                break;
        }
    }

    // 根据切图中心点坐标获取原图像素坐标和用印角度
    int ImgPro::STSeachSealPoint(
            const char* in_src_img_name,
            int in_x,
            int in_y,
            double in_angle,
            int &out_x,
            int &out_y,
            double &out_angle)
    {
        return SearchImgStampPointEx(
                in_src_img_name,
                in_x,
                in_y,
                in_angle,
                out_x,
                out_y,
                out_angle);
    }

    // 将原图用印像素坐标转换为设备(印控机)用印坐标(毫米)
    Point* ImgPro::GetSealCoord(int nX, int nY)
    {
        typedef float var;

        var dminx = 3;          //2//6
        var dminy = 60;         //56//60
        var dmaxx = 270;        //270//270
        var dmaxy = 250;        // 250//239
        var w = dmaxx - dminx;  //--印章校准位置最大X-印章校准位置最小X 物理=239
        var h = dmaxy - dminy;  //--印章校准位置最大Y-印章校准位置最小Y 物理=182
        var x0 = nX;            //--原件盖章X坐标
        var y0 = nY;            //--原件盖章Y坐标

        var x1 = 1914;          //--A4像素最小X坐标 像素 右上角坐标
        var y1 = 230;           //--A4像素最小Y坐标 像素

        var x2 = 208;           //--[===[A4像素最大X坐标 像素]===] 左下角坐标
        var y2 = 1442;          //--[===[A4像素最大Y坐标 像素]===]

        // 读取校准点数据
//         std::vector<struct Point*> VerifyPoists;
//         unsigned short buffer[10] = { 0 };
//         int ret = CalibrationPoint(buffer, 10);
//         if (ret == 0) {
//             for (int i = 0; i < 5; ++i) {
//                 struct Point* pt = new Point;
//                 pt->x = buffer[i];
//                 pt->y = buffer[i + 1];
//                 VerifyPoists.push_back(pt);
//             }
//         } else {
//             Log::WriteLog(LL_DEBUG, "未读取到校准点数据,使用系统默认值");
//         }
//
//         if (VerifyPoists.size() > 4 && VerifyPoists.at(1)->x != 65535 &&
//             VerifyPoists.at(1)->y != 65535) {
//             // 左上, 右上(1), 右下, 左下(3), 中间
//             x1 = VerifyPoists[1]->x;
//             y1 = VerifyPoists[1]->y;
//             x2 = VerifyPoists[3]->x;
//             y2 = VerifyPoists[3]->y;
//
//             for (int i = 0; i < VerifyPoists.size(); ++i) {
//                 delete VerifyPoists.at(i);
//             }
//         } else {
//             x1 = MC::SvrConfig::GetInst()->check_pt2_.x;
//             y1 = MC::SvrConfig::GetInst()->check_pt2_.y;
//             x2 = MC::SvrConfig::GetInst()->check_pt4_.x;;
//             y2 = MC::SvrConfig::GetInst()->check_pt4_.y;
//         }

        x1 = MC::SvrConfig::GetInst()->check_pt1_.x;
        y1 = MC::SvrConfig::GetInst()->check_pt1_.y;
        x2 = MC::SvrConfig::GetInst()->check_pt3_.x;;
        y2 = MC::SvrConfig::GetInst()->check_pt3_.y;

        // 比例计算
        var dRateX = (double)(fabs(x2 - x1) * 1000) / (double)(fabs(dmaxx - dminx) * 1000); // 7.85
        var dRateY = (double)(fabs(y2 - y1) * 1000) / (double)(fabs(dmaxy - dminy) * 1000); // 7.88
        var devX = (double)(fabs(x1 - nX) * 1000) / (double)(dRateX * 1000) + dminx;
        var devY = (double)(fabs(y1 - nY) * 1000) / (double)(dRateY * 1000) + dminy;
        int x = ceil(devX);
        int y = ceil(devY);

        bool bFix = false;
        if (x < 1) {
            x = 1;
            bFix = true;
        }
        if (x > 270) {
            x = 270;
            bFix = true;
        }
        if (y < 56) {
            y = 56;
            bFix = true;
        }
        if (y > 250) {
            y = 250;
            bFix = true;
        }

        if (bFix) {
            Log::WriteLog(LL_DEBUG, "修正设备用印位置, Device({2},{3})", nX, nY, x, y);
        }

        return new Point(x, y);
    }

    int ImgPro::CutImage(
            const std::string&  ori,
            std::string&        cut)
    {
        return CutImgEdgeEx(
                ori.c_str(),
                (char*)cut.c_str());
    }

    // 合并图片
    int ImgPro::MergeImage(
            const std::string& img1,
            const std::string& img2,
            const std::string& merged)
    {
        return Merge2Imgs(
                img1.c_str(),
                img2.c_str(),
                merged.c_str());
    }

    int ImgPro::GetModelTypeAnglePoint(
            const std::string&  img_path,
            char*               model_type,
            double&             outangle,
            int&                x,
            int&                y)
    {
        return RecoModelTypeAndAngleAndModelPointByImg(
                img_path.c_str(),
                model_type,
                outangle,
                x,
                y);
    }

    int ImgPro::IdentifyImage(
            const std::string&  cut_img,
            const std::string&  model_type,
            std::string&        out_model_type,
            std::string&        voucher_no,
            std::string&        trace_no,
            int&                x,
            int&                y,
            int&                angle) {
        return RecoModelTypeAndCodeAndAngleAndPointByImg(
            cut_img.c_str(),
            (char*)model_type.c_str(),
            (char*)out_model_type.c_str(),
            (char*)voucher_no.c_str(),
            (char*)trace_no.c_str(),
            x,
            y,
            angle);
    }

    int ImgPro::IdentifyArea(
            const std::string& img,
            int x,
            int y,
            int width,
            int height,
            int angle,
            std::string& result) {
        return RecoImgRectArea(
            img.c_str(),
            x,
            y,
            x + width,
            y + height,
            0.f,
            (char*)result.c_str(),
            image);
    }

    int ImgPro::SearchSrcImgStampPoint(
        const std::string& img,
        int in_x,
        int in_y,
        double in_angle,
        int& out_x,
        int& out_y,
        double& out_angle) {
        return SearchImgStampPointEx(
            img.c_str(),
            in_x,
            in_y,
            in_angle,
            out_x,
            out_y,
            out_angle);
    }

} // namespace MC
