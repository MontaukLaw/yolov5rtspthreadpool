
#ifndef RK3588_DEMO_LOGGING_H
#define RK3588_DEMO_LOGGING_H

// a logging wrapper so it can be easily replaced
#include <stdio.h>
#include "log4c.h"

// log level from low to high
// 0: no log
// 1: error
// 2: error, warning
// 3: error, warning, info
// 4: error, warning, info, debug
static int32_t g_log_level = 4;

// a printf wrapper so the msg can be formatted with %d %s, etc.

#define NN_LOG_ERROR(...)  LOGE(__VA_ARGS__)
#define NN_LOG_WARNING(...)  LOGW(__VA_ARGS__)
#define NN_LOG_INFO(...)  LOGI(__VA_ARGS__)
#define NN_LOG_DEBUG(...)  LOGD(__VA_ARGS__)

//#define NN_LOG_DEBUG(...)          \
//    do                             \
//    {                              \
//        if (g_log_level >= 4)      \
//        {                          \
//            printf("[NN_DEBUG] "); \
//            printf(__VA_ARGS__);   \
//            printf("\n");          \
//        }                          \
//    } while (0)

#endif // RK3588_DEMO_LOGGING_H
