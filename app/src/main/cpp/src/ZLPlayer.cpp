#include <api/include/mk_common.h>
#include <api/include/mk_player.h>
#include <android/native_window_jni.h>
#include "ZLPlayer.h"
#include "mpp_err.h"
#include "cv_draw.h"
// Yolov8ThreadPool *yolov8_thread_pool;   // 线程池

extern pthread_mutex_t windowMutex;     // 静态初始化 所
extern ANativeWindow *window;

void *rtps_process(void *arg) {
    ZLPlayer *player = (ZLPlayer *) arg;
    if (player) {
        player->process_video_rtsp();
    } else {
        LOGE("player is null");
    }
    return nullptr;
}

void *desplay_process(void *arg) {
    ZLPlayer *player = (ZLPlayer *) arg;
    if (player) {
        while (1) {
            player->display();
        }
    } else {
        LOGE("player is null");
    }
    return nullptr;
}

void ZLPlayer::setModelFile(char *data, int dataLen) {
    // 申请内存
    this->modelFileContent = new char[dataLen];
    // 复制内存
    memcpy(this->modelFileContent, data, dataLen);
    this->modelFileSize = dataLen;
}

ZLPlayer::ZLPlayer() {

    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source, data_source); // 把源 Copy给成员
    this->isStreaming = false;

    LOGD("create mpp");

    // 创建上下文
    memset(&app_ctx, 0, sizeof(rknn_app_context_t)); // 初始化上下文
    // app_ctx.job_cnt = 1;
    // app_ctx.result_cnt = 1;
    // app_ctx.mppDataThreadPool = new MppDataThreadPool();

    // yolov8_thread_pool = new Yolov8ThreadPool(); // 创建线程池
    // yolov8_thread_pool->setUpWithModelData(20, this->modelFileContent, this->modelFileSize);
    app_ctx.yolov5ThreadPool = new Yolov5ThreadPool(); // 创建线程池
    // app_ctx.mppDataThreadPool->setUpWithModelData(THREAD_POOL, this->modelFileContent, this->modelFileSize);

    // yolov8_thread_pool->setUp(model_path, 12);   // 初始化线程池

    // MPP 解码器
    if (app_ctx.decoder == nullptr) {
        MppDecoder *decoder = new MppDecoder();           // 创建解码器
        decoder->Init(264, 25, &app_ctx);          // 初始化解码器
        decoder->SetCallback(mpp_decoder_frame_callback); // 设置回调函数，用来处理解码后的数据
        app_ctx.decoder = decoder;                        // 将解码器赋值给上下文
    }
    // 启动rtsp线程
    pthread_create(&pid_rtsp, nullptr, rtps_process, this);
    // pthread_create(&pid_render, nullptr, desplay_process, this);
    // 读取视频流
    // process_video_rtsp(&app_ctx, "rtsp://192.168.1.159:554/stream1");
}

void API_CALL

on_track_frame_out(void *user_data, mk_frame frame) {
    rknn_app_context_t *ctx = (rknn_app_context_t *) user_data;
    // LOGD("on_track_frame_out ctx=%p\n", ctx);
    const char *data = mk_frame_get_data(frame);
    ctx->dts = mk_frame_get_dts(frame);
    ctx->pts = mk_frame_get_pts(frame);
    size_t size = mk_frame_get_data_size(frame);
    if (mk_frame_get_flags(frame) & MK_FRAME_FLAG_IS_KEY) {
        LOGD("Key frame size: %zu", size);
    } else if (MK_FRAME_FLAG_DROP_ABLE & mk_frame_get_flags(frame)) {
        LOGD("Drop able: %zu", size);
    } else if (MK_FRAME_FLAG_IS_CONFIG & mk_frame_get_flags(frame)) {
        LOGD("Config frame: %zu", size);
    } else if (MK_FRAME_FLAG_NOT_DECODE_ABLE & mk_frame_get_flags(frame)) {
        LOGD("Not decode able: %zu", size);
    } else {
        // LOGD("P-frame: %zu", size);
    }

    // LOGD("ctx->dts :%ld, ctx->pts :%ld", ctx->dts, ctx->pts);
    // LOGD("decoder=%p\n", ctx->decoder);
    ctx->decoder->Decode((uint8_t *) data, size, 0);
}

void API_CALL

on_mk_play_event_func(void *user_data, int err_code, const char *err_msg, mk_track tracks[],
                      int track_count) {
    rknn_app_context_t *ctx = (rknn_app_context_t *) user_data;
    if (err_code == 0) {
        // success
        LOGD("play success!");
        int i;
        // ctx->push_url = "rtmp://localhost/live/stream";
        // ctx->media = mk_media_create("__defaultVhost__", "live", "test", 0, 0, 0);
        for (i = 0; i < track_count; ++i) {
            if (mk_track_is_video(tracks[i])) {
                LOGD("got video track: %s", mk_track_codec_name(tracks[i]));
                // 监听track数据回调
                mk_track_add_delegate(tracks[i], on_track_frame_out, user_data);
            }
        }
    } else {
        printf("play failed: %d %s", err_code, err_msg);
    }
}

void API_CALL

on_mk_shutdown_func(void *user_data, int err_code, const char *err_msg, mk_track tracks[], int track_count) {
    printf("play interrupted: %d %s", err_code, err_msg);
}

// 函数指针的实现 实现渲染画面
void renderFrame(uint8_t *src_data, int width, int height, int src_line_size) {

    pthread_mutex_lock(&windowMutex);
    if (!window) {
        pthread_mutex_unlock(&windowMutex); // 出现了问题后，必须考虑到，释放锁，怕出现死锁问题
    }

    // 设置窗口的大小，各个属性
    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);

    // 他自己有个缓冲区 buffer
    ANativeWindow_Buffer window_buffer;

    // 如果我在渲染的时候，是被锁住的，那我就无法渲染，我需要释放 ，防止出现死锁
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;

        pthread_mutex_unlock(&windowMutex); // 解锁，怕出现死锁
        return;
    }

    // 填充[window_buffer]  画面就出来了  ==== 【目标 window_buffer】
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_linesize = window_buffer.stride * 4;

    for (int i = 0; i < window_buffer.height; ++i) {
        // 图：一行一行显示 [高度不用管，用循环了，遍历高度]
        // 通用的
        memcpy(dst_data + i * dst_linesize, src_data + i * src_line_size, dst_linesize); // OK的
    }

    // 数据刷新
    ANativeWindow_unlockAndPost(window); // 解锁后 并且刷新 window_buffer的数据显示画面
    pthread_mutex_unlock(&windowMutex);
}

void ZLPlayer::display() {
    // int queueSize = app_ctx.renderFrameQueue->size();
    // LOGD("app_ctx.renderFrameQueue.size() :%d", queueSize);

    // auto frameDataPtr = app_ctx.renderFrameQueue->pop();
    //    if (frameDataPtr == nullptr) {
    //        LOGD("frameDataPtr is null");
    //        return;
    //    }
    std::this_thread::sleep_until(nextRendTime);
    // renderFrame((uint8_t *) frameDataPtr->data, frameDataPtr->screenW, frameDataPtr->screenH, frameDataPtr->screenStride);
    // 释放内存
    // delete frameDataPtr->data;
    // frameDataPtr->data = nullptr;

    // 设置下一次执行的时间点
    nextRendTime = steady_clock::now() + milliseconds(30);

}

void ZLPlayer::get_detect_result() {
    std::vector <Detection> objects;
    // LOGD("decoder_callback Getting result count :%d", app_ctx.result_cnt);
    auto ret_code = app_ctx.yolov5ThreadPool->getTargetResultNonBlock(objects, app_ctx.result_cnt);
    if (ret_code == NN_SUCCESS) {

        // auto frameData = ;
        app_ctx.yolov5ThreadPool->getTargetImgResult(app_ctx.result_cnt);

        app_ctx.result_cnt++;
        LOGD("Get detect result counter:%d start display", app_ctx.result_cnt);

        // 加入队列
        // app_ctx.renderFrameQueue->push(frameData);

        // renderFrame((uint8_t *) frameData->data, frameData->screenW, frameData->screenH, frameData->screenStride);

        // 释放内存
        // delete frameData->data;
        // frameData->data = nullptr;

        // app_ctx.renderFrameQueue->push(frameData);

    } else if (NN_RESULT_NOT_READY == ret_code) {
        // LOGD("decoder_callback wait for result ready");
    }
}

int ZLPlayer::process_video_rtsp() {

    mk_config config;
    memset(&config, 0, sizeof(mk_config));
    config.log_mask = LOG_CONSOLE;
    mk_env_init(&config);
    mk_player player = mk_player_create();
    mk_player_set_on_result(player, on_mk_play_event_func, &app_ctx);
    mk_player_set_on_shutdown(player, on_mk_shutdown_func, &app_ctx);
    mk_player_play(player, rtsp_url);

    while (1) {
        // usleep(1000);
        // display();
        // display();
        get_detect_result();
    }

#if 0
    std::vector<Detection> objects;
    cv::Mat origin_mat = cv::Mat::zeros(height, width, CV_8UC3);

    // LOGD("enter any key to exit\n");
    while (1) {
        // LOGD("running\n");
        // sleep(1);
        usleep(1000);

        // 获取推理结果
        auto ret_code = yolov8_thread_pool->getTargetResultNonBlock(objects, result_cnt);
        auto ret_code = yolov8_thread_pool-> getTargetImgResult(objects, result_cnt);
        if (ret_code == NN_SUCCESS) {
            result_cnt++;
        }else{
            continue;
        }

        DrawDetections(origin_mat, objects);

        cv::cvtColor(origin_mat, origin_mat, cv::COLOR_RGB2RGBA);
        renderFrame(origin_mat.data, width, height, width * get_bpp_from_format(RK_FORMAT_RGBA_8888));

        gettimeofday(&end, NULL);

        double timeused = 1000 * (end.tv_sec - now.tv_sec) + (end.tv_usec - now.tv_usec) / 1000;
        // LOGD("Spent:%f", timeused);

        long frameGap = end.tv_sec * 1000 + end.tv_usec / 1000 - lastRenderTime.tv_usec / 1000 - lastRenderTime.tv_sec * 1000;

        LOGD("Frame gap :%ld\n", frameGap);

        gettimeofday(&lastRenderTime, NULL);

    }
#endif

    if (player) {
        mk_player_release(player);
    }
    return 0;
}

ZLPlayer::~ZLPlayer() {

}

static struct timeval lastRenderTime;

void ZLPlayer::mpp_decoder_frame_callback(void *userdata, int width_stride, int height_stride, int width, int height, int format, int fd, void *data) {
    rknn_app_context_t *ctx = (rknn_app_context_t *) userdata;
    struct timeval start;
    struct timeval end;
    struct timeval memCpyEnd;
    gettimeofday(&start, NULL);
    long frameGap = start.tv_sec * 1000 + start.tv_usec / 1000 - lastRenderTime.tv_usec / 1000 - lastRenderTime.tv_sec * 1000;
    // LOGD("decoded frame ctx->dts:%ld", ctx->dts);
    LOGD("mpp_decoder_frame_callback Frame gap :%ld\n", frameGap);
    gettimeofday(&lastRenderTime, NULL);

    // 12,441,600 3840x2160x3/2
    // int imgSize = width * height * get_bpp_from_format(RK_FORMAT_RGBA_8888);
#if 0
    auto frameData = std::make_shared<frame_data_t>();

    char *dstBuf = new char[imgSize]();
    memcpy(dstBuf, data, imgSize);

    gettimeofday(&memCpyEnd, NULL);
    frameGap = memCpyEnd.tv_sec * 1000 + memCpyEnd.tv_usec / 1000 - start.tv_usec / 1000 - start.tv_sec * 1000;
    LOGD("mpp_decoder_frame_callback Frame mem cpy spent :%ld\n", frameGap);

    frameData->dataSize = imgSize;
    frameData->screenStride = width * get_bpp_from_format(RK_FORMAT_YCbCr_420_SP);  // 解码出来的格式就是nv12
    frameData->data = dstBuf;
    frameData->screenW = width;
    frameData->screenH = height;
    frameData->heightStride = height_stride;
    frameData->widthStride = width_stride;
    frameData->frameFormat = RK_FORMAT_YCbCr_420_SP;
    frameData->frameId = ctx->job_cnt;

    ctx->job_cnt++;

    // 放入显示队列
    ctx->renderFrameQueue->push(frameData);

    LOGD("mpp_decoder_frame_callback task list size :%d", ctx->mppDataThreadPool->get_task_size());

    // 放入线程池, 进行并行推理
    ctx->mppDataThreadPool->submitTask(frameData);

    gettimeofday(&end, NULL);
    frameGap = end.tv_sec * 1000 + end.tv_usec / 1000 - start.tv_usec / 1000 - start.tv_sec * 1000;
    LOGD("mpp_decoder_frame_callback Frame spent :%ld\n", frameGap);

    return;
#endif

    int dstImgSize = width_stride * height_stride * get_bpp_from_format(RK_FORMAT_RGBA_8888);
    LOGD("img size is %d", dstImgSize);
    // img size is 33177600 1080p: 8355840
    char *dstBuf = new char[dstImgSize]();
    // rga_change_color_async(width_stride, height_stride, RK_FORMAT_YCbCr_420_SP, (char *) data,
    // width_stride, height_stride, RK_FORMAT_RGBA_8888, dstBuf);

    rga_change_color(width_stride, height_stride, RK_FORMAT_YCbCr_420_SP, (char *) data,
                     width_stride, height_stride, RK_FORMAT_RGBA_8888, dstBuf);

    auto frameData = std::make_shared<frame_data_t>();
    frameData->dataSize = dstImgSize;
    frameData->screenStride = width * get_bpp_from_format(RK_FORMAT_RGBA_8888);
    frameData->data = dstBuf;
    frameData->screenW = width;
    frameData->screenH = height;
    frameData->heightStride = height_stride;
    frameData->widthStride = width_stride;
    frameData->frameFormat = RK_FORMAT_RGBA_8888;

    // LOGD(">>>>>  frame id:%d", frameData->frameId);
    // LOGD("mpp_decoder_frame_callback task list size :%d", ctx->mppDataThreadPool->get_task_size());

    // 放入显示队列
    // ctx->renderFrameQueue->push(frameData);

    frameData->frameId = ctx->job_cnt;
    int detectPoolSize = ctx->yolov5ThreadPool->get_task_size();

    LOGD("detectPoolSize :%d", detectPoolSize);

    // ctx->mppDataThreadPool->submitTask(frameData);
    // ctx->job_cnt++;
    // 如果frameData->frameId为奇数
    ctx->frame_cnt++;
    if (ctx->frame_cnt % 2 == 1) {
        // if (detectPoolSize < MAX_TASK) {
        // 放入线程池, 进行并行推理
        ctx->mppDataThreadPool->submitTask(frameData);
        ctx->job_cnt++;
    } else {
        // 直接释放
        delete frameData->data;
        frameData->data = nullptr;
    }

}

#if 0

void mpp_decoder_frame_callback_good_display(void *userdata, int width_stride, int height_stride, int width, int height, int format, int fd, void *data) {
    rknn_app_context_t *ctx = (rknn_app_context_t *) userdata;
    struct timeval end;
    gettimeofday(&end, NULL);
    long frameGap = end.tv_sec * 1000 + end.tv_usec / 1000 -
                    lastRenderTime.tv_usec / 1000 - lastRenderTime.tv_sec * 1000;
    // LOGD("decoded frame ctx->dts:%ld", ctx->dts);
    LOGD("mpp_decoder_frame_callback Frame gap :%ld\n", frameGap);
    gettimeofday(&lastRenderTime, NULL);

    rga_buffer_t origin;

    int imgSize = width * height * get_bpp_from_format(RK_FORMAT_RGBA_8888);
    // char *dstBuf = (char *) malloc(imgSize);
    // memset(dstBuf, 0, imgSize);
    // std::unique_ptr<char[]> dstBuf(new char[imgSize]());

    char *dstBuf = new char[imgSize]();
    rga_change_color_async(width_stride, height_stride, RK_FORMAT_YCbCr_420_SP, (char *) data,
                           width, height, RK_FORMAT_RGBA_8888, dstBuf);

    // usleep(1000 * 80);

#if 0

    origin = wrapbuffer_fd(fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
    cv::Mat origin_mat = cv::Mat::zeros(height, width, CV_8UC3);
    rga_buffer_t rgb_img = wrapbuffer_virtualaddr((void *) origin_mat.data, width, height, RK_FORMAT_RGB_888);
    imcopy(origin, rgb_img);


    // yolov8_thread_pool->submitTask(origin_mat, ctx->job_cnt++);
    yolov8_thread_pool->submitTask(origin_mat, ctx->job_cnt);
    std::vector<Detection> objects;

    // 获取推理结果
    // auto ret_code = yolov8_thread_pool->getTargetResultNonBlock(objects, ctx->result_cnt);
    auto ret_code = yolov8_thread_pool->getTargetResultNonBlock(objects, ctx->job_cnt);
    if (ret_code == NN_SUCCESS) {
        ctx->result_cnt++;
    }
    LOGD("ctx->result_cnt:%d", ctx->result_cnt);



    detect_result_group_t detect_result_group;
    memset(&detect_result_group, 0, sizeof(detect_result_group_t));
    detect_result_group.count = objects.size();
    uint8_t idx;
    for (idx = 0; idx < objects.size(); idx++) {
        LOGD("objects[%d].classId: %d\n", idx, objects[idx].class_id);
        LOGD("objects[%d].prop: %f\n", idx, objects[idx].confidence);
        LOGD("objects[%d].class name: %s\n", idx, objects[idx].className.c_str());
        // int left;
        // int right;
        // int top;
        // int bottom;
        detect_result_group.results[idx].box.left = objects[idx].box.x;
        detect_result_group.results[idx].box.right = objects[idx].box.x + objects[idx].box.width;
        detect_result_group.results[idx].box.top = objects[idx].box.y;
        detect_result_group.results[idx].box.bottom = objects[idx].box.y + objects[idx].box.height;
        detect_result_group.results[idx].classId = objects[idx].class_id;
        detect_result_group.results[idx].prop = objects[idx].confidence;
        strcpy(detect_result_group.results[idx].name, objects[idx].className.c_str());
    }
#endif

    // frame_data_t *frameData = new frame_data_t();
    auto frameData = std::make_shared<frame_data_t>();
    frameData->dataSize = imgSize;
    frameData->screenStride = width * get_bpp_from_format(RK_FORMAT_RGBA_8888);
    frameData->data = dstBuf;
    frameData->screenW = width;
    frameData->screenH = height;
    frameData->heightStride = height_stride;
    frameData->widthStride = width_stride;

    ctx->renderFrameQueue->push(frameData);
    LOGD("Now render frame queue size is :%d", ctx->renderFrameQueue->size());

    //
    //    rga_buffer_t origin;
    //    rga_buffer_t src;
    //    int mpp_frame_fd = 0;
    //
    //    // 复制到另一个缓冲区，避免修改mpp解码器缓冲区
    //    // 使用的是RK RGA的格式转换：YUV420SP -> RGB888
    //    origin = wrapbuffer_fd(fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
    //    src = wrapbuffer_fd(mpp_frame_fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
    //    cv::Mat origin_mat = cv::Mat::zeros(height, width, CV_8UC3);
    //    rga_buffer_t rgb_img = wrapbuffer_virtualaddr((void *) origin_mat.data, width, height, RK_FORMAT_RGB_888);
    //    imcopy(origin, rgb_img);
    //
    //    LOGD("task size is:%d\n", yolov8_thread_pool->get_task_size());
    //    // 提交推理任务给线程池
    //    yolov8_thread_pool->submitTask(origin_mat, job_cnt++);
    //    std::vector<Detection> objects;
    // yolov8_thread_pool->submitTask(origin_mat, job_cnt++);
}

// 解码后的数据回调函数
void ZLPlayer::mpp_decoder_frame_callback(void *userdata, int width_stride, int height_stride, int width, int height, int format, int fd, void *data) {

    //    LOGD("width_stride :%d height_stride :%d width :%d height :%d format :%d\n",
    //         width_stride, height_stride, width, height, format);
    // LOGD("mpp_decoder_frame_callback\n");
    struct timeval now;
    struct timeval end;
    gettimeofday(&now, NULL);
    rknn_app_context_t *ctx = (rknn_app_context_t *) userdata;

    int ret = 0;
    static int frame_index = 0;
    frame_index++;

    void *mpp_frame = NULL;
    int mpp_frame_fd = 0;
    void *mpp_frame_addr = NULL;
    int enc_data_size;

    rga_buffer_t origin;
    rga_buffer_t src;

    // 复制到另一个缓冲区，避免修改mpp解码器缓冲区
    // 使用的是RK RGA的格式转换：YUV420SP -> RGB888
    origin = wrapbuffer_fd(fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
    src = wrapbuffer_fd(mpp_frame_fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
    cv::Mat origin_mat = cv::Mat::zeros(height, width, CV_8UC3);
    rga_buffer_t rgb_img = wrapbuffer_virtualaddr((void *) origin_mat.data, width, height, RK_FORMAT_RGB_888);
    imcopy(origin, rgb_img);

    static int job_cnt = 0;
    static int result_cnt = 0;
    LOGD("task size is:%d\n", yolov8_thread_pool->get_task_size());
    // 提交推理任务给线程池
    yolov8_thread_pool->submitTask(origin_mat, job_cnt++);
    std::vector<Detection> objects;
    // 获取推理结果
    auto ret_code = yolov8_thread_pool->getTargetResultNonBlock(objects, result_cnt);
    if (ret_code == NN_SUCCESS) {
        result_cnt++;
    }

    uint8_t idx;
    for (idx = 0; idx < objects.size(); idx++) {
        LOGD("objects[%d].classId: %d\n", idx, objects[idx].class_id);
        LOGD("objects[%d].prop: %f\n", idx, objects[idx].confidence);
        LOGD("objects[%d].class name: %s\n", idx, objects[idx].className.c_str());
        // int left;
        // int right;
        // int top;
        // int bottom;
    }

    DrawDetections(origin_mat, objects);
    // imcopy(rgb_img, src);

    // LOGD("result_cnt: %d\n", result_cnt);
    cv::cvtColor(origin_mat, origin_mat, cv::COLOR_RGB2RGBA);
    renderFrame(origin_mat.data, width, height, width * get_bpp_from_format(RK_FORMAT_RGBA_8888));

    gettimeofday(&end, NULL);

    double timeused = 1000 * (end.tv_sec - now.tv_sec) + (end.tv_usec - now.tv_usec) / 1000;
    // LOGD("Spent:%f", timeused);

    long frameGap = end.tv_sec * 1000 + end.tv_usec / 1000 - lastRenderTime.tv_usec / 1000 - lastRenderTime.tv_sec * 1000;

    LOGD("Frame gap :%ld\n", frameGap);

    gettimeofday(&lastRenderTime, NULL);

}
#endif

//void ZLPlayer::setRenderCallback(RenderCallback renderCallback_) {
//    this->renderCallback = renderCallback_;
//}
