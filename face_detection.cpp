#include "face_detection.h"
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "utility_tool.h"

face_detection::face_detection()
{
    std::string face_file("./haarcascade_frontalface_alt2.xml");
    if(boost::filesystem::exists(face_file)){
        mp_cascade = std::make_shared<cv::CascadeClassifier>();
        mp_cascade->load(face_file);
    }else{
        LOG_ERROR("找不到对应的文件检测模型文件:"<<face_file);
    }
}


bool face_detection::detection_face(std::vector<info_face_ptr> & faces, unsigned char* p_data, int width, int height, bool flag_dectection){
    if(!mp_cascade){
        return false;
    }

    static int c = 0;
    if(0 != (c++ % 25)){
        return true;
    }

    cv::Mat bgr(cv::Size(width, height), CV_8UC3);
    bgr.data = p_data;
    cv::Mat gray;
    gray.create(bgr.size(), bgr.type());
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Rect> rect;
    mp_cascade->detectMultiScale(gray, rect, 1.1, 3, 0);
    for (auto& r : rect)
    {
        cv::rectangle(bgr, r, CV_RGB(255, 0, 0), 2);
    }
    return true;
}
