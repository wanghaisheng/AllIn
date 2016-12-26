#include "stdafx.h"
#include "log.h"
#include "parse.h"
#include "base_api.h"

namespace MC {

    // 根据切图用印点像素坐标获取盖章点基于切图中心点位置, 像素坐标
    void CaculatePos(
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

    typedef DWORD (*my_SearchImgStampPointEx)(
        const char* in_src_img_name, 
        int in_x, 
        int in_y, 
        double in_angle,
        int &out_x, 
        int &out_y, 
        double &out_angle);

    // 根据切图中心点坐标获取原图像素坐标和用印角度
    int STSeachSealPoint(
        const char* in_src_img_name,
        int in_x,
        int in_y,
        double in_angle,
        int &out_x,
        int &out_y,
        double &out_angle)
    {
        std::string path;
        GetMoudulePath(path);
        path = path.append("ABC.STDZ.Device.STAMP.ImgPro.dll");
        HINSTANCE dllins = LoadLibrary(path.c_str());
        if (dllins != NULL) {
            my_SearchImgStampPointEx func_ptr = 
                (my_SearchImgStampPointEx)GetProcAddress(dllins, "SearchImgStampPointEx");

            return func_ptr(
                in_src_img_name,
                in_x,
                in_y,
                in_angle,
                out_x,
                out_y,
                out_angle);
        }

        FreeLibrary(dllins);
        return MC::EC_FAIL;
    }

    // 将原图用印像素坐标转换为设备(印控机)用印坐标(毫米)
    Point* GetSealCoord(int nX, int nY)
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

    typedef DWORD (*my_CutImgEdge)(const char* in_src_img_name, char *out_dest_img_name);

    int CutImage(
        const std::string&  ori,
        std::string&        cut)
    {
        std::string path;
        GetMoudulePath(path);
        path = path.append("ABC.STDZ.Device.STAMP.ImgPro.dll");
        HINSTANCE dllins = LoadLibrary(path.c_str());
        if (dllins != NULL) {
            my_CutImgEdge func_ptr =
                (my_CutImgEdge)GetProcAddress(dllins, "CutImgEdge");

            return func_ptr(
                ori.c_str(),
                (char*)cut.c_str());
        }

        FreeLibrary(dllins);
        return MC::EC_FAIL;
    }

    typedef DWORD (*my_Merge2Imgs)(
        const char* src_file_name1, 
        const char* src_file_name2, 
        const char* dst_file_name);

    // 合并图片
    int MergeImage(
        const std::string& img1,
        const std::string& img2,
        const std::string& merged)
    {
        std::string path;
        GetMoudulePath(path);
        path = path.append("ABC.STDZ.Device.STAMP.ImgPro.dll");
        HINSTANCE dllins = LoadLibrary(path.c_str());
        if (dllins != NULL) {
            my_Merge2Imgs func_ptr =
                (my_Merge2Imgs)GetProcAddress(dllins, "Merge2Imgs");

            return func_ptr(
                img1.c_str(),
                img2.c_str(),
                merged.c_str());
        }

        FreeLibrary(dllins);
        return MC::EC_FAIL;
    }
    
    typedef DWORD (*my_RecoModelTypeAndAngleAndModelPointByImg)(
        const char* in_src_img_name, 
        char *out_model_type,
        double &outangle,
        int &x,
        int &y);

    int GetModelTypeAnglePoint(
        const std::string&  img_path,
        std::string&        model_type,
        double&             outangle,
        int&                x,
        int&                y)
    {
        std::string path;
        GetMoudulePath(path);
        path = path.append("ABC.STDZ.Device.STAMP.ImgPro.dll");
        HINSTANCE dllins = LoadLibrary(path.c_str());
        if (dllins != NULL) {
            my_RecoModelTypeAndAngleAndModelPointByImg func_ptr =
                (my_RecoModelTypeAndAngleAndModelPointByImg)GetProcAddress(
                    dllins, "RecoModelTypeAndAngleAndModelPointByImg");

            return func_ptr(
                img_path.c_str(),
                (char*)model_type.c_str(),
                outangle,
                x,
                y);
        }

        FreeLibrary(dllins);
        return MC::EC_FAIL;
    }

    typedef DWORD (*my_RecoModelTypeAndCodeAndAngleAndPointByImg)(
        const char* in_cut_img_name, 
        char *in_model_type,
        char *out_model_type,
        char* out_Vocher_Number, 
        char* out_trace_code, 
        int &x, 
        int &y,
        int &outangle);

    int IdentifyImage(
        const std::string&  cut_img,
        const std::string&  model_type,
        std::string&        out_model_type,
        std::string&        voucher_no,
        std::string&        trace_no,
        int&                x,
        int&                y,
        int&                angle) {
        std::string path;
        GetMoudulePath(path);
        path = path.append("ABC.STDZ.Device.STAMP.ImgPro.dll");
        HINSTANCE dllins = LoadLibrary(path.c_str());
        if (dllins != NULL) {
            my_RecoModelTypeAndCodeAndAngleAndPointByImg func_ptr =
                (my_RecoModelTypeAndCodeAndAngleAndPointByImg)GetProcAddress(
                dllins, "RecoModelTypeAndCodeAndAngleAndPointByImg");

            return func_ptr(
                cut_img.c_str(),
                (char*)model_type.c_str(),
                (char*)out_model_type.c_str(),
                (char*)voucher_no.c_str(),
                (char*)trace_no.c_str(),
                x,
                y,
                angle);
        }

        FreeLibrary(dllins);
        return MC::EC_FAIL;
    }

    enum FontLib {
        image = 0,
        eng = 1
    };

    typedef DWORD (*my_RecoImgRectArea)(
        const char* in_dest_img_name,
        int left, 
        int top,
        int right, 
        int bottom,
        double threshold,
        char *ocrresult, 
        FontLib lib);

    int IdentifyArea(
        const std::string& img,
        int x,
        int y,
        int width,
        int height,
        int angle,
        std::string& result) {
        std::string path;
        GetMoudulePath(path);
        path = path.append("ABC.STDZ.Device.STAMP.ImgPro.dll");
        HINSTANCE dllins = LoadLibrary(path.c_str());
        if (dllins != NULL) {
            my_RecoImgRectArea func_ptr =
                (my_RecoImgRectArea)GetProcAddress(
                dllins, "RecoImgRectArea");

            return func_ptr(
                img.c_str(),
                x,
                y,
                x + width,
                y + height,
                0.f,
                (char*)result.c_str(),
                image);
        }

        FreeLibrary(dllins);
        return MC::EC_FAIL;
    }

} // namespace MC
