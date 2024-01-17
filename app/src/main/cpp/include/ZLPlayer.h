#ifndef AIBOX_ZLPLAYER_H
#define AIBOX_ZLPLAYER_H

#include "safe_queue.h"
#include "util.h"
#include "rknn_api.h"
#include <unistd.h>
#include "rk_mpi.h"
#include "im2d.h"
#include "rga.h"
#include "RgaUtils.h"
#include "im2d.hpp"
#include "rga_utils.h"
#include "mpp_decoder.h"
#include "yolov5_thread_pool.h"
#include "display_queue.h"

typedef struct g_rknn_app_context_t {
    FILE *out_fp;
    MppDecoder *decoder;
    Yolov5ThreadPool *yolov5ThreadPool;
    RenderFrameQueue *renderFrameQueue;
    // MppEncoder *encoder;
    // mk_media media;
    // mk_pusher pusher;
    const char *push_url;
    uint64_t pts;
    uint64_t dts;

    int job_cnt;
    int result_cnt;
    int frame_cnt;

} rknn_app_context_t;

class ZLPlayer {

private:
    char *data_source = 0; // 指针 请赋初始值
    bool isStreaming = 0; // 是否播放
    pthread_t pid_rtsp = 0;
    pthread_t pid_render = 0;
    char *modelFileContent = 0;
    int modelFileSize = 0;

    std::chrono::steady_clock::time_point nextRendTime;

public:
    // static RenderCallback renderCallback;
    rknn_app_context_t app_ctx;
    // char *rtsp_url = "rtsp://192.168.1.10:554/stream1";
    char *rtsp_url = "rtsp://192.168.1.159:554/stream1";
    // char *rtsp_url = "rtsp://192.168.1.155:554/stream1";

    // ZLPlayer(const char *data_source, JNICallbackHelper *helper);
    ZLPlayer(char *modelFileData, int modelDataLen);

    ~ZLPlayer();

    static void mpp_decoder_frame_callback(void *userdata, int width_stride, int height_stride, int width, int height, int format, int fd, void *data);

    int process_video_rtsp();

    // int process_video_rtsp(rknn_app_context_t *ctx, const char *url);
    void setModelFile(char *data, int dataLen);

    // void setRenderCallback(RenderCallback renderCallback_);

    void display();

    void get_detect_result();
};

#endif //AIBOX_ZLPLAYER_H
