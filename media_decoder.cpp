#include "media_decoder.h"
#include "utility_tool.h"
#include <vector>


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
    AVFormatContext *p_context = nullptr;
    auto ret = avformat_open_input(&p_context, url.c_str(), nullptr, nullptr);
    if (0 != ret)
    {
        LOG_ERROR("打开url失败;  错误代码:" << ret << "; 路径:" << url);
        return ;
    }
    ret = avformat_find_stream_info(p_context, nullptr);
    if (0 > ret)
    {
        LOG_ERROR("获取流信息失败; 错误代码:" << ret);
        return;
    }

    AVCodec *pi_code = nullptr;
    auto index_video = av_find_best_stream(p_context, AVMEDIA_TYPE_VIDEO, -1, -1, &(pi_code), 0);
    if (0 > index_video)
    {
        LOG_ERROR("获取视频索引失败:" << index_video);
        return;
    }
    auto p_video_stream = p_context->streams[index_video];
    auto p_video_code_ctx = avcodec_alloc_context3(pi_code);
    ret = avcodec_parameters_to_context(p_video_code_ctx, p_video_stream->codecpar);
    if(0 > ret){
        return;
    }
    ret = avcodec_open2(p_video_code_ctx, pi_code, nullptr);
    if(0 > ret){
        return;
    }

    AVPixelFormat pix_fmt_out = AV_PIX_FMT_RGBA;
    AVFrame *p_frame_yuv = av_frame_alloc();
    int num_yuv = av_image_get_buffer_size(p_video_code_ctx->pix_fmt, p_video_code_ctx->width, p_video_code_ctx->height, 1);
    uint8_t* p_data_yuv = static_cast<uint8_t *>(av_malloc(static_cast<std::size_t>(num_yuv)*sizeof(uint8_t)));
    av_image_fill_arrays(p_frame_yuv->data, p_frame_yuv->linesize, p_data_yuv, p_video_code_ctx->pix_fmt, p_video_code_ctx->width, p_video_code_ctx->height, 1);

    AVFrame *p_frame_rgb = av_frame_alloc();
    int num_rgb = av_image_get_buffer_size(pix_fmt_out, width_out, height_out, 1);
    uint8_t* p_data_rgb = static_cast<uint8_t *>(av_malloc(static_cast<std::size_t>(num_rgb)*sizeof(uint8_t)));
    av_image_fill_arrays(p_frame_rgb->data, p_frame_rgb->linesize, p_data_rgb, pix_fmt_out, width_out, height_out, 1);

    struct SwsContext *p_sws_context = nullptr;
    p_sws_context = sws_getCachedContext(p_sws_context, p_video_code_ctx->width, p_video_code_ctx->height, p_video_code_ctx->pix_fmt,
        width_out, height_out, pix_fmt_out, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if(nullptr == p_sws_context){
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
        int re = avcodec_send_packet(p_video_code_ctx ,&pkt);
        if (0 > ret)
        {
            av_packet_unref(&pkt);
            continue;
        }
        while(!*p_stop){
            re = avcodec_receive_frame(p_video_code_ctx, p_frame_yuv);
            if (re != 0)
            {
                break;
            }

            if(nullptr != p_sws_context){
                auto h = sws_scale(p_sws_context, p_frame_yuv->data, p_frame_yuv->linesize, 0, p_video_code_ctx->height, p_frame_rgb->data, p_frame_rgb->linesize);
                if(0 < h && fun){
                    auto p_info = std::make_shared<info_data>();
                    p_info->p_data = p_data_rgb;
                    p_info->width = width_out;
                    p_info->height = height_out;
                    p_info->data_max = static_cast<std::size_t>(num_rgb);
                    fun(p_info);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
            }else{
                LOG_ERROR("获取缩放上下文失败; 宽度:"<<p_frame_yuv->width<<"; 高度:"<<p_frame_yuv->height<<"; 帧格式:"<<static_cast<AVPixelFormat>(p_frame_yuv->format));
                break;
            }
        }
        av_packet_unref(&pkt);
    }
    if(nullptr != p_sws_context){
        sws_freeContext(p_sws_context);
        p_sws_context = nullptr;
    }
    if(nullptr != p_frame_rgb){
        av_frame_free(&p_frame_rgb);
        p_frame_rgb = nullptr;
    }
    if(nullptr != p_data_rgb){
        av_free(p_data_rgb);
        p_data_rgb = nullptr;
    }
    if(nullptr != p_frame_yuv){
        av_frame_free(&p_frame_yuv);
        p_frame_yuv = nullptr;
    }
    if(nullptr != p_data_yuv){
        av_free(p_data_yuv);
        p_data_yuv = nullptr;
    }
    if(nullptr != p_context){
        avformat_close_input(&p_context);
        p_context = nullptr;
    }
}
