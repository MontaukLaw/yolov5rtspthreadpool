#include "rga_utils.h"

int rga_change_color_async(int src_width, int src_height, int src_format, char *src_buf,
                           int dst_width, int dst_height, int dst_format, char *dst_buf) {
    int ret = 0;

    // LOGD("src_width: %d, src_height: %d,  dst_width: %d, dst_height: %d, ",
    // src_width, src_height, dst_width, dst_height);

    int src_buf_size, dst_buf_size;
    im_rect src_rect;
    im_rect dst_rect;

    rga_buffer_t src_img, dst_img;
    rga_buffer_handle_t src_handle, dst_handle;

    memset(&dst_rect, 0, sizeof(dst_rect));
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&src_img, 0, sizeof(src_img));
    memset(&dst_img, 0, sizeof(dst_img));

    src_buf_size = src_width * src_height * get_bpp_from_format(src_format);
    dst_buf_size = dst_width * dst_height * get_bpp_from_format(dst_format);

    // src_buf = (char *) malloc(src_buf_size);
    // dst_buf = (char *) malloc(dst_buf_size);

    /* fill image data */
    // memset(src_buf, 0xaa, src_buf_size);
    // memset(dst_buf, 0x80, dst_buf_size);

    src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
    dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
    if (src_handle == 0 || dst_handle == 0) {
        LOGD("importbuffer failed!\n");
        goto release_buffer;
    }

    src_img.handle = src_handle;
    src_img.width = src_width;
    src_img.height = src_height;
    src_img.format = src_format;
    src_img.wstride = src_width;
    src_img.hstride = src_height;

    dst_img.handle = dst_handle;
    dst_img.width = dst_width;
    dst_img.height = dst_height;
    dst_img.format = dst_format;
    dst_img.wstride = dst_width;
    dst_img.hstride = dst_height;

    src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
    dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

    // 获取任务
    im_job_handle_t job_handle;
    job_handle = imbeginJob();
    if (job_handle <= 0) {
        LOGD("job begin failed![%d], %s\n", job_handle, imStrError());
        goto release_buffer;
    }
    ret = imcheck(src_img, dst_img, {}, dst_rect);
    if (IM_STATUS_NOERROR != ret) {
        LOGD("%d %d, check error! %s \n", ret, __LINE__, imStrError((IM_STATUS) ret));
        imcancelJob(job_handle);
        goto release_buffer;
    }
    ret = improcessTask(job_handle, src_img, dst_img, {}, {}, dst_rect, {}, NULL, IM_SYNC);
    if (ret != IM_STATUS_SUCCESS) {
        // LOGD("%s job[%d] add left task failed, %s\n", LOG_TAG, job_handle, imStrError((IM_STATUS)ret));
        imcancelJob(job_handle);
        goto release_buffer;
    }

    ret = imendJob(job_handle);
    if (ret != IM_STATUS_SUCCESS) {
        // LOGD("%s job[%d] running failed, %s\n", LOG_TAG, job_handle, imStrError((IM_STATUS)ret));
        // LOGD("When job failed, virt ram add is %p data : %p\n", rgbImg.vir_addr, data);
        goto release_buffer;
    }
    // write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

    release_buffer:
    if (src_handle)
        releasebuffer_handle(src_handle);
    if (dst_handle)
        releasebuffer_handle(dst_handle);

    return ret;

}

int rga_change_color(int src_width, int src_height, int src_format, char *src_buf,
                     int dst_width, int dst_height, int dst_format, char *dst_buf) {
    int ret = 0;

    // LOGD("src_width: %d, src_height: %d,  dst_width: %d, dst_height: %d, ",
        // src_width, src_height, dst_width, dst_height);

    int src_buf_size, dst_buf_size;

    rga_buffer_t src_img, dst_img;
    rga_buffer_handle_t src_handle, dst_handle;

    memset(&src_img, 0, sizeof(src_img));
    memset(&dst_img, 0, sizeof(dst_img));

    src_buf_size = src_width * src_height * get_bpp_from_format(src_format);
    dst_buf_size = dst_width * dst_height * get_bpp_from_format(dst_format);

    // src_buf = (char *) malloc(src_buf_size);
    // dst_buf = (char *) malloc(dst_buf_size);

    /* fill image data */
    // memset(src_buf, 0xaa, src_buf_size);
    // memset(dst_buf, 0x80, dst_buf_size);

    src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
    dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
    if (src_handle == 0 || dst_handle == 0) {
        LOGD("importbuffer failed!\n");
        goto release_buffer;
    }

    src_img.handle = src_handle;
    src_img.width = src_width;
    src_img.height = src_height;
    src_img.format = src_format;
    src_img.wstride = src_width;
    src_img.hstride = src_height;

    dst_img.handle = dst_handle;
    dst_img.width = dst_width;
    dst_img.height = dst_height;
    dst_img.format = dst_format;
    dst_img.wstride = dst_width;
    dst_img.hstride = dst_height;

    ret = imcvtcolor(src_img, dst_img, src_format, dst_format);
    if (ret != IM_STATUS_SUCCESS) {
        LOGD("running failed, %s\n", imStrError((IM_STATUS) ret));
    }

    // write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

    release_buffer:
    if (src_handle)
        releasebuffer_handle(src_handle);
    if (dst_handle)
        releasebuffer_handle(dst_handle);

    return ret;
}

int rga_resize(int src_width, int src_height, int src_format, char *src_buf,
               int dst_width, int dst_height, int dst_format, char *dst_buf) {

    im_rect src_rect;
    im_rect dst_rect;
    memset(&dst_rect, 0, sizeof(dst_rect));
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));

    // rga_buffer_t src = wrapbuffer_virtualaddr((void *) img_rgb.data, img.cols, img.rows, RK_FORMAT_RGB_888);
    // rga_buffer_t dst = wrapbuffer_virtualaddr((void *) tensor.data, width, height, RK_FORMAT_RGB_888);
    rga_buffer_t src = wrapbuffer_virtualaddr((void *) src_buf, src_width, src_height, src_format);
    rga_buffer_t dst = wrapbuffer_virtualaddr((void *) dst_buf, dst_width, dst_height, dst_format);

    int ret = imcheck(src, dst, src_rect, dst_rect);
    if (IM_STATUS_NOERROR != ret) {
        LOGD("%d, check error! %s", __LINE__, imStrError((IM_STATUS) ret));
        return (-1);
    }

    imresize(src, dst);
    return 0;
}

/*
 * Find larger side and scale smaller side to fit the larger side
 * Then fill the board color
 */
int rga_add_boarder(int src_width, int src_height, int src_format, char *src_buf,
                    int dst_width, int dst_height, int dst_format, char *dst_buf, float wh_ratio) {

    float img_width = src_width;
    float img_height = src_height;

    int letterbox_width = 0;
    int letterbox_height = 0;

    LetterBoxInfo info;
    int padding_hor = 0;
    int padding_ver = 0;

    if (img_width / img_height > wh_ratio) {
        info.hor = false;
        letterbox_width = img_width;
        letterbox_height = img_width / wh_ratio;
        info.pad = (letterbox_height - img_height) / 2.f;
        padding_hor = 0;
        padding_ver = info.pad;
    } else {
        info.hor = true;
        letterbox_width = img_height * wh_ratio;
        letterbox_height = img_height;
        info.pad = (letterbox_width - img_width) / 2.f;
        padding_hor = info.pad;
        padding_ver = 0;
    }

    // LOGD("letterbox_width: %d, letterbox_height: %d, padding_hor: %d, padding_ver: %d",
         // letterbox_width, letterbox_height, padding_hor, padding_ver);

    // rga add border
    // img_letterbox = cv::Mat::zeros(letterbox_height, letterbox_width, CV_8UC3);

    im_rect src_rect;
    im_rect dst_rect;
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));

    // NN_LOG_INFO("img size: %d, %d", img.cols, img.rows);

    // rga_buffer_t src = wrapbuffer_virtualaddr((void *)img.data, img.cols, img.rows, RK_FORMAT_RGB_888);
    // rga_buffer_t dst = wrapbuffer_virtualaddr((void *)img_letterbox.data, img_letterbox.cols, img_letterbox.rows, RK_FORMAT_RGB_888);

    rga_buffer_t src = wrapbuffer_virtualaddr((void *) src_buf, src_width, src_height, src_format);
    rga_buffer_t dst = wrapbuffer_virtualaddr((void *) dst_buf, dst_width, dst_height, dst_format);

    int ret = imcheck(src, dst, src_rect, dst_rect);
    if (IM_STATUS_NOERROR != ret) {
        LOGD("%d, check error! %s", __LINE__, imStrError((IM_STATUS) ret));
        return -1;
    }

    immakeBorder(src, dst, padding_ver, padding_ver, padding_hor, padding_hor, 0, 0, 0);

    return 0;
}

static double __get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

int rga_letter_box(int src_width, int src_height, int src_format, char *src_buf,
                   int dst_width, int dst_height, int dst_format, char *dst_buf, float wh_ratio) {
    struct timeval start_time, stop_time, resize_time, change_color_time;
    gettimeofday(&start_time, NULL);
    // Step 1. resize
    // 原来是1920x1080, 直接先resize到640x360
    // 色彩为RGBA888
    int mid_width = 640;
    int mid_height = 360;
    char *mid_buf = (char *) malloc(mid_width * mid_height * 4);
    if (!mid_buf) {
        LOGD("malloc mid_buf failed\n");
        return -1;
    }

    rga_resize(src_width, src_height, src_format, src_buf,
               mid_width, mid_height, src_format, mid_buf);

    gettimeofday(&resize_time, NULL);
    double time_interval = (__get_us(resize_time) - __get_us(start_time)) / 1000;

    // LOGD("resize spent: %f\n", time_interval);

    // Step 2. 转色彩空间为RGB888
    char *rgb_buf = (char *) malloc(mid_width * mid_height * 3);
    if (!rgb_buf) {
        LOGD("malloc rgb_buf failed\n");
        return -1;
    }
    rga_change_color(mid_width, mid_height, src_format, mid_buf,
                     mid_width, mid_height, dst_format, rgb_buf);

    gettimeofday(&change_color_time, NULL);
    time_interval = (__get_us(change_color_time) - __get_us(resize_time)) / 1000;
    // LOGD("change color spent: %f\n", time_interval);

    // Step 3. letter box
    rga_add_boarder(mid_width, mid_height, dst_format, rgb_buf,
                    dst_width, dst_height, dst_format, dst_buf, wh_ratio);


    gettimeofday(&stop_time, NULL);
    time_interval = (__get_us(stop_time) - __get_us(start_time)) / 1000;
    LOGD("Pre-process spent: %f\n", time_interval);

    // 释放资源
    free(mid_buf);
    free(rgb_buf);

    return 0;
}
