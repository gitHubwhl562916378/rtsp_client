extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include "opencv2/opencv.hpp"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "MultiRTSPClient.h"
#include "Queue.h"
unsigned rtspClientCount = 0;
volatile char eventLoopWatchVariable = 0;
Queue<AVPacket*> g_frames_queue;

int main(int, char**) {//rtsp://192.168.2.253:8000/test.264 rtsp://192.168.2.38:5554/2
    std::vector<std::string> urls{"rtsp://192.168.2.38:5554/2"};
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

    std::for_each(urls.begin(),urls.end(),[&](const std::string &url){
        MultiRTSPClient *client = new MultiRTSPClient(*env,url.data(),1,"multi_rtsp_client");
        rtspClientCount++;
        client->sendDescribeCommand(continueAfterDESCRIBE);
    });

    std::thread decode_thr([&]{
        av_register_all();
        SwsContext *img_convert_ctx;
        AVFrame *decoded_frame = av_frame_alloc(), *rgb_frame = av_frame_alloc();

        AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if(!codec){
            std::cout << "codec not find" << std::endl;
            return;
        }
        AVCodecContext *c = avcodec_alloc_context3(codec);
        if(avcodec_open2(c, codec, nullptr) < 0){
            std::cout << "could not open codec" << std::endl;
            return;
        }

        bool isFirstFrame = true;
        int frameFinished = 0;
        uint8_t* m_buffer;
        while (true) {
             AVPacket* pkt = g_frames_queue.Pop();
             avcodec_decode_video2(c, decoded_frame, &frameFinished, pkt);
             if(frameFinished){
                 std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
#if 0  //解码1080p图片 ffmpeg 转出到YUV缓存I帧耗时12ms,其它帧耗时2－3ms,直接从解码的AVFrame里面拷贝，耗时0－1ms
                 if(isFirstFrame){
                     int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P,c->width,c->height);
                     m_buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
                     avpicture_fill((AVPicture*)rgb_frame,m_buffer,AV_PIX_FMT_YUV420P,c->width,c->height);
                     img_convert_ctx = sws_getContext(c->width,c->height,c->pix_fmt,c->width,c->height,AV_PIX_FMT_YUV420P,SWS_FAST_BILINEAR,nullptr,nullptr,nullptr);
                     isFirstFrame = false;
                 }

                 sws_scale(img_convert_ctx,(const uint8_t *const*)decoded_frame->data,decoded_frame->linesize,0,c->height,rgb_frame->data,rgb_frame->linesize);
                 std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
                 std::chrono::milliseconds use_time_interval = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                 std::cout << "use time: " << use_time_interval.count() << std::endl;

                 cv::Mat frame(3 * c->height / 2, c->width, CV_8UC1, m_buffer);
                 cv::cvtColor(frame, frame, CV_YUV2BGR_I420);
#else
                 if(isFirstFrame){
                     int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P,c->width,c->height);
                     m_buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
                     isFirstFrame = false;
                 }

                 int bytes = 0; //yuv有data有3块内存分别拷，nv12只有2块内存分别拷
                 for(int i = 0; i < c->height; i++){ //将y分量拷贝
                     ::memcpy(m_buffer + bytes,decoded_frame->data[0] + decoded_frame->linesize[0] * i, c->width);
                     bytes += c->width;
                 }
                 int uv = c->height >> 1;
                 for(int i = 0; i < uv; i++){ //将u分量拷贝
                     ::memcpy(m_buffer + bytes,decoded_frame->data[1] + decoded_frame->linesize[1] * i, c->width >> 1);
                     bytes += c->width >> 1;
                 }
                 for(int i = 0; i < uv; i++){ //将v分量拷贝
                     ::memcpy(m_buffer + bytes,decoded_frame->data[2] + decoded_frame->linesize[2] * i, c->width >> 1);
                     bytes += c->width >> 1;
                 }
                 std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
                 std::chrono::milliseconds use_time_interval = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                 std::cout << "use time: " << use_time_interval.count() << std::endl;

                 cv::Mat frame(3 * c->height / 2, c->width, CV_8UC1, m_buffer);
                 cv::cvtColor(frame, frame, CV_YUV2BGR_I420);
#endif
                 cv::imshow("rtsVieo",frame);
                 cv::waitKey(1);
             }

             av_free_packet(pkt);
             delete pkt;
        }

        sws_freeContext(img_convert_ctx);
    });

    env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
    env->reclaim(); env = NULL;
    delete scheduler; scheduler = NULL;

    if(decode_thr.joinable()){
        decode_thr.join();
    }
}
