#ifndef DERRY_PLAYER_LOG4C_H
#define DERRY_PLAYER_LOG4C_H

#include <android/log.h>

#define TAG "BKAI"

// ...代表可传递任意内容
// __VA_ARGS__ 是内部的一个宏，此宏可以把我们要输出打印的内容 给 __android_log_print 来打印
// __VA_ARGS__ 代表 ...的可变参数
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG,  __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG,  __VA_ARGS__);
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG,  __VA_ARGS__);
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG ,__VA_ARGS__)

#endif //DERRY_PLAYER_LOG4C_H
