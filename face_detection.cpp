#include "face_detection.h"
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "utility_tool.h"

face_detection::face_detection()
{
    // 加载opencv提供的人脸检测模型，识别率比较低
    std::string face_file("./haarcascade_frontalface_alt2.xml");
    if(boost::filesystem::exists(face_file)){
        mp_cascade = std::make_shared<cv::CascadeClassifier>();
        mp_cascade->load(face_file);
    }else{
        LOG_ERROR("找不到对应的文件检测模型文件:"<<face_file);
    }
}


bool face_detection::detection_face(std::vector<info_face_ptr> & faces, unsigned char* p_data, int width, int height, bool flag_dectection){
    // 这里p_data的图像格式是AV_PIX_FMT_BGR24
    if(!mp_cascade){
        return false;
    }

    // 太耗资源了，所以这里25帧后才检测一次
    static int c = 0;
    if(0 != (c++ % 25)){
        return true;
    }

    // 转换成灰度图像，消耗资源相对比较少
    cv::Mat bgr(cv::Size(width, height), CV_8UC3);
    bgr.data = p_data;
    cv::Mat gray;
    gray.create(bgr.size(), bgr.type());
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

    // 人脸检测
    std::vector<cv::Rect> rect;
    mp_cascade->detectMultiScale(gray, rect, 1.1, 3, 0);
    for (auto& r : rect)
    {
        cv::rectangle(bgr, r, CV_RGB(255, 0, 0), 2);
        auto p_info = std::make_shared<info_face>();
        p_info->p_data = p_data;
        p_info->x = r.x;
        p_info->y = r.y;
        p_info->width = r.width;
        p_info->height = r.height;
        faces.push_back(p_info);
    }
    return true;
}
