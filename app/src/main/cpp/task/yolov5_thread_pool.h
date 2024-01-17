
#ifndef RK3588_DEMO_YOLOV5_THREAD_POOL_H
#define RK3588_DEMO_YOLOV5_THREAD_POOL_H

#include "yolov5.h"

#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>

class Yolov5ThreadPool {
private:
    std::queue <std::pair<int, cv::Mat>> tasks;
    std::vector <std::shared_ptr<Yolov5>> yolov5_instances;
    std::map<int, std::vector<Detection>> results;
    std::map<int, cv::Mat> img_results;
    std::vector <std::thread> threads;
    std::mutex mtx1;
    std::mutex mtx2;
    std::condition_variable cv_task, cv_result;
    bool stop;

    void worker(int id);

public:
    Yolov5ThreadPool();

    ~Yolov5ThreadPool();

    void stopAll(); // 停止所有线程
    nn_error_e setUp(std::string &model_path, int num_threads = 12);

    nn_error_e submitTask(const cv::Mat &img, int id);

    nn_error_e getTargetResult(std::vector <Detection> &objects, int id);

    nn_error_e getTargetResultNonBlock(std::vector <Detection> &objects, int id);

    nn_error_e getTargetImgResult(int id);
    
    int get_task_size() {
        return tasks.size();
    }
};

#endif // RK3588_DEMO_YOLOV5_THREAD_POOL_H