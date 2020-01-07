#ifndef FACE_RECOGNITION_H
#define FACE_RECOGNITION_H

#include "utility_tool.h"
#include <vector>
#include <string>
#include <memory>
#include<opencv2/face.hpp>

class info_recognition{
public:
    int index = 0;
    std::string name;
    std::vector<std::string> files;
};
typedef std::shared_ptr<info_recognition> info_recognition_ptr;

class face_recognition
{
public:
    face_recognition();

    virtual void start();
    virtual bool train(unsigned char *p_data, int width, int heigth);
    virtual bool find_and_save_face(const std::string& folder_desc, const std::string& folder_src);
    virtual bool find_face_info(std::vector<info_recognition_ptr>& infos, const std::string& folder);

protected:
    virtual bool find_file_from_folder(std::vector<std::string>& files, const std::string& folder, const std::string& extend = "");
    virtual bool find_folder_from_folder(std::vector<std::string>& all_folders, const std::string& folder);

    cv::Ptr<cv::face::FaceRecognizer> mp_model;
    std::shared_ptr<cv::CascadeClassifier> mp_cascade;
    std::map<int, std::string> m_index_to_name;
};
typedef std::shared_ptr<face_recognition> face_recognition_ptr;

#endif // FACE_RECOGNITION_H
