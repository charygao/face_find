#include "face_recognition.h"
#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

using namespace cv;
using namespace cv::face;

face_recognition::face_recognition()
{

}

void face_recognition::start(){
    try{
        Ptr<LBPHFaceRecognizer> p_model = LBPHFaceRecognizer::create();
        // 获取训练图片集合
        std::vector<info_recognition_ptr> infos;
        if(!find_face_info(infos, "./face")){
            return;
        }
        std::vector<Mat> images;
        std::vector<int> labels;
        for(auto& p_info : infos){
            for(auto& f : p_info->files){
                // 这里记得是IMREAD_GRAYSCALE，即灰度图片，不然会报错
                images.push_back(imread(f, cv::IMREAD_GRAYSCALE));
                labels.push_back(p_info->index);
            }
            m_index_to_name.insert(std::make_pair(p_info->index, p_info->name));
        }
        // 训练模型
        p_model->train(images, labels);
        mp_model = p_model;

        // 加载opencv提供的人脸检测模型，识别率比较低
        std::string face_file("./haarcascade_frontalface_alt2.xml");
        if(boost::filesystem::exists(face_file)){
            mp_cascade = std::make_shared<cv::CascadeClassifier>();
            mp_cascade->load(face_file);
        }else{
            LOG_ERROR("找不到对应的文件检测模型文件:"<<face_file);
        }
    }catch(const std::exception& e){
        LOG_ERROR("发生错误:"<<e.what());
    }
}

bool face_recognition::train(unsigned char *p_data, int width, int height){
    try {
        if(!mp_model || !mp_cascade){
            return false;
        }
        // 原始图片数据转成灰度图片
        cv::Mat bgr(cv::Size(width, height), CV_8UC3);
        bgr.data = p_data;
        cv::Mat gray;
        gray.create(bgr.size(), bgr.type());
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

        // 先人脸检测，再人脸识别
        std::vector<cv::Rect> rect;
        mp_cascade->detectMultiScale(gray, rect, 1.1, 3, 0);
        for (auto& r : rect)
        {
            bool flag_find_face = false;
            std::string name;
            // 复制出检测出来的人脸，然后转换成灰度图片，不转换会报错
            cv::Mat desc;
            bgr(r).copyTo(desc);
            cv::Mat gray_desc;
            gray_desc.create(desc.size(), desc.type());
            cv::cvtColor(desc, gray_desc, cv::COLOR_BGR2GRAY);
            int index = -1;
            double confidence = 0.0;
            // 实际总会返回一个检测结果，置信度暂时不知道怎么使用
            mp_model->predict(gray_desc, index, confidence);
            if(0 > index){

            }else{
                auto iter = m_index_to_name.find(index);
                if(m_index_to_name.end() == iter){
                    LOG_WARN("找不到对应人脸信息；序号:"<<index);
                }else{
                    LOG_INFO("找到人脸；名称:"<<iter->second<<"; 可信度:"<<confidence<<"; 序号:"<<index);
                    name = iter->second;
                    flag_find_face = true;
                }
            }
            if(flag_find_face){
                cv::rectangle(bgr, r, CV_RGB(0, 255, 0), 4);
                cv::putText(bgr, name, Point(r.x + 0.5 * r.width, r.y - 5), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(0, 255, 0));
            }else{
                cv::rectangle(bgr, r, CV_RGB(255, 0, 0), 2);
            }
        }
    } catch(const std::exception& e){
        LOG_ERROR("发生错误:"<<e.what());
    }

    return true;
}


bool face_recognition::find_and_save_face(const std::string& folder_desc, const std::string& folder_src){
    std::vector<std::string> files;
    if(!find_file_from_folder(files, folder_src)){
        return false;
    }
    cv::CascadeClassifier cascade;
    cascade.load("./haarcascade_frontalface_alt2.xml");
    for(auto& p : files){
        cv::Mat src = cv::imread(p);
        if(nullptr == src.data){
            LOG_ERROR("文件无法正常读取:"<<p);
            continue;
        }
        cv::Mat gray;
        gray.create(src.size(), src.type());
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

        // 人脸检测
        std::vector<cv::Rect> rect;
        cascade.detectMultiScale(gray, rect, 1.1, 3, 0);
        if(rect.empty()){
            LOG_WARN("没有检测到人脸:"<<p);
            continue;
        }
        int index = 0;
        for (auto& r : rect)
        {
            cv::Mat desc;
            src(r).copyTo(desc);
            imwrite((boost::format("%s/%d.png") % folder_desc % (index++)).str(), desc);
        }
    }
    return true;
}
bool face_recognition::find_face_info(std::vector<info_recognition_ptr>& infos, const std::string& folder){
    std::vector<std::string> all_folders;
    if(!find_folder_from_folder(all_folders, boost::filesystem::system_complete(folder).string())){
        return false;
    }
    int index = 1;
    for(auto& d : all_folders){
        std::vector<std::string> files;
        if (!find_file_from_folder(files, d)) {
            continue;
        }
        auto path = boost::filesystem::system_complete(d);
        auto parent_folder = path.parent_path().string();
        auto name = path.string().substr(parent_folder.size() + 1);
        auto p_info = std::make_shared<info_recognition>();
        p_info->index = index++;
        p_info->name = name;
        for(auto& f : files){
            p_info->files.push_back(f);
        }
        infos.push_back(p_info);
    }
    return true;
}

bool face_recognition::find_file_from_folder(std::vector<std::string>& files, const std::string& folder, const std::string& extend){
    boost::filesystem::path path_folder(folder);
    if(!boost::filesystem::exists(path_folder)){
        LOG_ERROR("文件夹路径不存在:"<<folder);
        return false;
    }
    boost::filesystem::directory_iterator end;
    for(boost::filesystem::directory_iterator iter(path_folder); iter != end; ++iter){
        auto path = iter->path();
        if (boost::filesystem::is_directory(path)) {
            continue;
        }
        if(!extend.empty() && extend != path.extension()){
            continue;
        }
        files.push_back(path.string());
    }
    return true;
}

bool face_recognition::find_folder_from_folder(std::vector<std::string>& all_folders, const std::string& folder){
    boost::filesystem::path path_folder(folder);
    if(!boost::filesystem::exists(path_folder)){
        LOG_ERROR("文件夹路径不存在:"<<folder);
        return false;
    }
    boost::filesystem::directory_iterator end;
    for(boost::filesystem::directory_iterator iter(path_folder); iter != end; ++iter){
        auto path = iter->path();
        if (!boost::filesystem::is_directory(path)) {
            continue;
        }
        if("." == path.string() || ".." == path.string()){
            continue;
        }
        all_folders.push_back(path.string());
    }
    return true;
}
