#ifndef __RGA_UTILS_H_
#define __RGA_UTILS_H_

#include "rga.h"
#include "im2d.h"
#include "rga.h"
#include "RgaUtils.h"
#include "im2d.hpp"
#include <string.h>
#include "util.h"
#include <unistd.h>
#include <malloc.h>

struct LetterBoxInfo {
    bool hor;
    int pad;
};

int rga_add_boarder(int src_width, int src_height, int src_format, char *src_buf,
                    int dst_width, int dst_height, int dst_format, char *dst_buf, float wh_ratio);

int rga_resize(int src_width, int src_height, int src_format, char *src_buf,
               int dst_width, int dst_height, int dst_format, char *dst_buf);

int rga_change_color(int src_width, int src_height, int src_format, char *src_buf,
                     int dst_width, int dst_height, int dst_format, char *dst_buf);

int rga_letter_box(int src_width, int src_height, int src_format, char *src_buf,
                   int dst_width, int dst_height, int dst_format, char *dst_buf, float wh_ratio);

int rga_change_color_async(int src_width, int src_height, int src_format, char *src_buf,
                           int dst_width, int dst_height, int dst_format, char *dst_buf);

#endif
