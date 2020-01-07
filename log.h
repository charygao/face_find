#ifndef LOG_H__
#define LOG_H__

#include <string>
#include <sstream>
#include <thread>


/// @brief 日志输出函数
/// @param info 日志等级字符串
/// @param msg 日志内容
void log_write(const std::string& info, const std::string& msg);

/// @brief 获取文件绝对路径中的文件名
/// @param file 绝对路径
/// @return 文件名
const char* get_file_name(const char* file);

/// @brief 创建统一日志消息内容
#define CREATE_LOG_MSG(MSG) std::stringstream abc123;abc123<<"["<<std::this_thread::get_id()<<"]["<<get_file_name(__FILE__)<<"::"<<__FUNCTION__<<"@"<<__LINE__<<"]"<<MSG

/// @brief 输出调试日志
#define LOG_DEBUG(MSG) {CREATE_LOG_MSG(MSG);log_write("debug", abc123.str());}

/// @brief 输出警告日志
#define LOG_WARN(MSG) {CREATE_LOG_MSG(MSG);log_write("warn", abc123.str());}

/// @brief 输出错误日志
#define LOG_ERROR(MSG) {CREATE_LOG_MSG(MSG);log_write("error", abc123.str());}

/// @brief 输出通知日志
#define LOG_INFO(MSG) {CREATE_LOG_MSG(MSG);log_write("info", abc123.str());}





#endif //LOG_H__
