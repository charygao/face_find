#ifndef FACE_DETECTION_H
#define FACE_DETECTION_H

#include "utility_tool.h"
#include <memory>
#include <vector>
#include <opencv2/opencv.hpp>

class info_face{
public:
    unsigned char* p_data = nullptr;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};
typedef std::shared_ptr<info_face>  info_face_ptr;

class face_detection
{
public:
    face_detection();

    bool detection_face(std::vector<info_face_ptr> & faces, unsigned char *p_data, int width, int heigth, bool flag_dectection = true);

protected:
    std::shared_ptr<cv::CascadeClassifier> mp_cascade;
};
typedef std::shared_ptr<face_detection> face_detection_ptr;

#endif // FACE_DETECTION_H
