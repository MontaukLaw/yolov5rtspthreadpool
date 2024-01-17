#ifndef MY_YOLOV5_RTSP_THREAD_POOL_USER_COMM_H
#define MY_YOLOV5_RTSP_THREAD_POOL_USER_COMM_H

#include "mpp_decoder.h"

typedef struct g_frame_data_t {
    char *data;
    long dataSize;
    int screenStride;
    int screenW;
    int screenH;
    int widthStride;
    int heightStride;
    int frameId;
    int frameFormat;
} frame_data_t;

#endif //MY_YOLOV5_RTSP_THREAD_POOL_USER_COMM_H
