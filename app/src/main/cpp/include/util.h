#ifndef MY_APPLICATION_FFMPEG_PLAYER_KT_UTIL_H
#define MY_APPLICATION_FFMPEG_PLAYER_KT_UTIL_H

#include <android/log.h>
#include <jni.h>
#include <pthread.h>
#include <string.h>

#define THREAD_MAIN 1
#define THREAD_CHILD 2

#define FFMPEG_CAN_NOT_OPEN_URL 1
#define FFMPEG_CAN_NOT_FIND_STREAMS 2
#define FFMPEG_FIND_DECODER_FAIL 3
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
#define FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL 6
#define FFMPEG_OPEN_DECODER_FAIL 7
#define FFMPEG_NO_MEDIA 8

#define MODEL_IMAGE_HEIGHT 640
#define MODEL_IMAGE_WIDTH 640
#define MODEL_INOUT_CHANNEL 3
#define MODEL_TOTAL_INPUT_DATA_SIZE 1228800
#define MODEL_INPUT_NUM 1
#define MODEL_OUTPUT_NUM 3

#define IMAGE_SCALE 3

// 原始图像大小
#define ORIGINAL_WIDTH 1920
#define ORIGINAL_HEIGHT 1080
#define ORIGINAL_CHANNELS 3
#define RGB_CHANNELS 3

// 目标图像大小
#define TARGET_WIDTH 640
#define TARGET_HEIGHT 360

#define OUTPUT_WIDTH 640
#define OUTPUT_HEIGHT 640

// 灰色边的大小
#define BORDER_SIZE 140

// 宏函数
#define DELETE(object) if (object) { delete object; object = 0;}

// 定义函数指针
typedef void (*RenderCallback)(uint8_t *, int, int, int);

typedef struct {
    unsigned char r, g, b, a;
} PixelRGBA;

// 定义一个RGB像素的结构体
typedef struct {
    unsigned char r, g, b;
} PixelRGB;

#endif //MY_APPLICATION_FFMPEG_PLAYER_KT_UTIL_H
