#include "media_decoder.h"
#include "utility_tool.h"
#include <vector>
#include "face_recognition.h"
#include <opencv2/opencv.hpp>

#define FREE_SWS(P) {if(nullptr != P){sws_freeContext(P);P = nullptr;}}
#define FREE_FRAME(P) {if(nullptr != P){av_frame_free(&P);P = nullptr;}}
#define FREE_DATA(P) {if(nullptr != P){av_free(P);P = nullptr;}}

media_decoder::media_decoder(const std::string& url, fun_type fun):m_url(url), m_fun(fun)
{

}

media_decoder::~media_decoder(){

}

void media_decoder::start(){
    mp_flag_stop = std::make_shared<std::atomic_bool>(false);
    m_thread = std::thread(std::bind(media_decoder::handle_media, m_url, m_fun, mp_flag_stop));
}

void media_decoder::stop(){
    mp_flag_stop && (*mp_flag_stop = true);
    m_thread.join();
}

void media_decoder::handle_media(std::string url, fun_type fun, std::shared_ptr<std::atomic_bool> p_stop){
    try {
        work(url, fun, 960, 540, p_stop);
    } catch (const std::exception& e) {
        LOG_ERROR("发生错误:"<<e.what());
    }
}

void media_decoder::work(std::string url, fun_type fun, int width_out, int height_out, std::shared_ptr<std::atomic_bool> p_stop){
    auto p_recognition = std::make_shared<face_recognition>();
    p_recognition->start();
    AVFormatContext *p_context = nullptr;
    // 打开视频
    auto ret = avformat_open_input(&p_context, url.c_str(), nullptr, nullptr);
    if (0 != ret)
    {
        LOG_ERROR("打开url失败;  错误代码:" << ret << "; 路径:" << url);
        return ;
    }
    // 获取视频信息
    ret = avformat_find_stream_info(p_context, nullptr);
    if (0 > ret)
    {
        LOG_ERROR("获取流信息失败; 错误代码:" << ret);
        return;
    }

    // 查找视频流
    AVCodec *pi_code = nullptr;
    auto index_video = av_find_best_stream(p_context, AVMEDIA_TYPE_VIDEO, -1, -1, &(pi_code), 0);
    if (0 > index_video)
    {
        LOG_ERROR("获取视频索引失败:" << index_video);
        return;
    }
    auto p_video_stream = p_context->streams[index_video];
    auto p_video_code_ctx = avcodec_alloc_context3(pi_code);
    // 复制视频流中相关参数到视频上下文，不然p_video_code_ctx某些参数会丢失
    ret = avcodec_parameters_to_context(p_video_code_ctx, p_video_stream->codecpar);
    if(0 > ret){
        return;
    }
    ret = avcodec_open2(p_video_code_ctx, pi_code, nullptr);
    if(0 > ret){
        return;
    }

    // 申请了三个Frame，一个用来存储解码后的视频帧，一个用来存储OpenCV的帧AV_PIX_FMT_BGR24，一个存储AV_PIX_FMT_RGBA用来显示

    AVFrame *p_frame_yuv = av_frame_alloc();
    int num_yuv = av_image_get_buffer_size(p_video_code_ctx->pix_fmt, p_video_code_ctx->width, p_video_code_ctx->height, 1);
    uint8_t* p_data_yuv = static_cast<uint8_t *>(av_malloc(static_cast<std::size_t>(num_yuv)*sizeof(uint8_t)));
    av_image_fill_arrays(p_frame_yuv->data, p_frame_yuv->linesize, p_data_yuv, p_video_code_ctx->pix_fmt, p_video_code_ctx->width, p_video_code_ctx->height, 1);

    AVPixelFormat pix_fmt_bgr = AV_PIX_FMT_BGR24;
    int width_bgr = p_video_code_ctx->width, height_bgr = p_video_code_ctx->height;
    AVFrame *p_frame_bgr = av_frame_alloc();
    int num_bgr = av_image_get_buffer_size(pix_fmt_bgr, width_bgr, height_bgr, 1);
    uint8_t* p_data_bgr = static_cast<uint8_t *>(av_malloc(static_cast<std::size_t>(num_bgr)*sizeof(uint8_t)));
    av_image_fill_arrays(p_frame_bgr->data, p_frame_bgr->linesize, p_data_bgr, pix_fmt_bgr, width_bgr, height_bgr, 1);

    AVPixelFormat pix_fmt_out = AV_PIX_FMT_RGBA;
    AVFrame *p_frame_out = av_frame_alloc();
    int num_out = av_image_get_buffer_size(pix_fmt_out, width_out, height_out, 1);
    uint8_t* p_data_out = static_cast<uint8_t *>(av_malloc(static_cast<std::size_t>(num_out)*sizeof(uint8_t)));
    av_image_fill_arrays(p_frame_out->data, p_frame_out->linesize, p_data_out, pix_fmt_out, width_out, height_out, 1);

    // 获取图像转换相关对象
    struct SwsContext *p_sws_context_bgr = nullptr;
    p_sws_context_bgr = sws_getCachedContext(p_sws_context_bgr, p_video_code_ctx->width, p_video_code_ctx->height, p_video_code_ctx->pix_fmt,
        width_bgr, height_bgr, pix_fmt_bgr, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if(nullptr == p_sws_context_bgr){
        return;
    }
    struct SwsContext *p_sws_context_out = nullptr;
    p_sws_context_out = sws_getCachedContext(p_sws_context_out, width_bgr, height_bgr, pix_fmt_bgr,
        width_out, height_out, pix_fmt_out, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if(nullptr == p_sws_context_out){
        return;
    }
    while(!*p_stop){
        AVPacket pkt;
        ret = av_read_frame(p_context, &pkt);
        if (0 > ret){
            break;
        }
        if (pkt.stream_index != index_video){
            continue;
        }
        // 这里使用了新版的解码函数，avcodec_send_packet后不一定能avcodec_receive_frame到帧，涉及到I、P、B帧的解码流程
        int re = avcodec_send_packet(p_video_code_ctx ,&pkt);
        if (0 > ret)
        {
            av_packet_unref(&pkt);
            continue;
        }
        while(!*p_stop){
            // 要反复调用avcodec_receive_frame，直到无法获取到帧
            re = avcodec_receive_frame(p_video_code_ctx, p_frame_yuv);
            if (re != 0)
            {
                break;
            }

            if(nullptr != p_sws_context_bgr){
                auto h = sws_scale(p_sws_context_bgr, p_frame_yuv->data, p_frame_yuv->linesize, 0, p_video_code_ctx->height, p_frame_bgr->data, p_frame_bgr->linesize);
                if(0 < h && fun){
                    p_recognition->train(p_data_bgr, width_bgr, height_bgr);

                    // 需要将AV_PIX_FMT_BGR24转换为AV_PIX_FMT_RGBA，因为QImage需要这种格式
                    h = sws_scale(p_sws_context_out, p_frame_bgr->data, p_frame_bgr->linesize, 0, height_bgr, p_frame_out->data, p_frame_out->linesize);

                    auto p_info = std::make_shared<info_data>();
                    p_info->p_data = p_data_out;
                    p_info->width = width_out;
                    p_info->height = height_out;
                    p_info->data_max = static_cast<std::size_t>(num_out);
                    fun(p_info);
                }
                // 如果不延迟，因为解码的速度很快，显示又是依赖于解码，会导致播放速度非常快，这里不应该写死，应该按照视频流的time_base来计算延迟
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
            }else{
                LOG_ERROR("获取缩放上下文失败; 宽度:"<<p_frame_yuv->width<<"; 高度:"<<p_frame_yuv->height<<"; 帧格式:"<<static_cast<AVPixelFormat>(p_frame_yuv->format));
                break;
            }
        }
        // 释放数据包
        av_packet_unref(&pkt);
    }

    // 清理相关对象
    FREE_SWS(p_sws_context_bgr);
    FREE_SWS(p_sws_context_out);
    FREE_FRAME(p_frame_out);
    FREE_DATA(p_data_out);
    FREE_FRAME(p_frame_bgr);
    FREE_DATA(p_data_bgr);
    FREE_FRAME(p_frame_yuv);
    FREE_DATA(p_data_yuv);
    if(nullptr != p_context){
        avformat_close_input(&p_context);
        p_context = nullptr;
    }
}
