#include "face_detection.h"
#include <opencv2/opencv.hpp>

face_detection::face_detection()
{

}


std::vector<info_face_ptr> face_detection::detection_face(unsigned char* p_data, int width, int height, bool flag_dectection){
    std::vector<info_face_ptr> faces;
    cv::Mat bgr(cv::Size(width, height), CV_8UC3);
    bgr.data = p_data;
    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGRA2RGBA);
    return faces;
}
