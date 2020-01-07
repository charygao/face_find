#ifndef MEDIA_DECODER_H
#define MEDIA_DECODER_H

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>


extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class info_data{
public:
    unsigned char* p_data = nullptr;
    int width = 0;
    int height = 0;
    std::size_t data_max = 0;
};
typedef std::shared_ptr<info_data> info_data_ptr;

class media_decoder
{
public:
    typedef std::function<void (info_data_ptr)> fun_type;

    media_decoder(const std::string& url, fun_type fun);
    virtual ~media_decoder();
    virtual void start();
    virtual void stop();
protected:
    static void handle_media(std::string url, fun_type fun, std::shared_ptr<std::atomic_bool> p_stop);
    static void work(std::string url, fun_type fun, int width_out, int height_out, std::shared_ptr<std::atomic_bool> p_stop);

    std::string m_url;
    fun_type m_fun;
    std::thread m_thread;
    std::shared_ptr<std::atomic_bool> mp_flag_stop;
    uint8_t* mp_data_rgb = nullptr;
};
typedef std::shared_ptr<media_decoder> media_decoder_ptr;

#endif // MEDIA_DECODER_H
