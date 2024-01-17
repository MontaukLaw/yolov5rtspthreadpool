
#include "yolov5_thread_pool.h"
#include "cv_draw.h"
#include "sys/time.h"

void Yolov5ThreadPool::worker(int id) {
    while (!stop) {
        // std::pair<int, cv::Mat> task;
        std::shared_ptr<frame_data_t> taskFrameData;
        std::shared_ptr<Yolov5> instance = yolov5_instances[id];
        {
            std::unique_lock<std::mutex> lock(mtx1);
            cv_task.wait(lock, [&] { return !tasks.empty() || stop; });

            if (stop) {
                return;
            }

            taskFrameData = tasks.front();
            tasks.pop();
        }

        std::vector<Detection> detections;
        struct timeval start, end;
        gettimeofday(&start, NULL);
        instance->RunWithFrameData(taskFrameData, detections);
        // instance->Run(task.second, detections);
        gettimeofday(&end, NULL);
        float time_use = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        LOGD("thread %d, time_use: %f ms\n", id, time_use);
        {
            std::lock_guard<std::mutex> lock(mtx2);
            results.insert({taskFrameData->frameId, detections});
            // DrawDetections(task.second, detections);
            // img_results.insert({task.first, task.second});
            img_results.insert({taskFrameData->frameId, taskFrameData});
            // cv_result.notify_one();
        }
    }
}


nn_error_e Yolov5ThreadPool::setUpWithModelData(int num_threads, char *modelData, int modelSize) {
    // 遍历线程数量，创建模型实例，放入vector
    // 这些线程加载的模型是同一个
    for (size_t i = 0; i < num_threads; ++i) {
        std::shared_ptr<Yolov5> yolov5 = std::make_shared<Yolov5>();
        yolov5->LoadModelWithData(modelData, modelSize);
        yolov5_instances.push_back(yolov5);
        usleep(1000);
    }

    // 遍历线程数量，创建线程
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(&Yolov5ThreadPool::worker, this, i);
    }
    return NN_SUCCESS;
}


nn_error_e Yolov5ThreadPool::setUp(std::string &model_path, int num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
        std::shared_ptr<Yolov5> yolov5 = std::make_shared<Yolov5>();
        yolov5->LoadModel(model_path.c_str());
        yolov5_instances.push_back(yolov5);
    }
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(&Yolov5ThreadPool::worker, this, i);
    }
    return NN_SUCCESS;
}

Yolov5ThreadPool::Yolov5ThreadPool() { stop = false; }

Yolov5ThreadPool::~Yolov5ThreadPool() {
    stop = true;
    cv_task.notify_all();
    for (auto &thread: threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

nn_error_e Yolov5ThreadPool::submitTask(const std::shared_ptr<frame_data_t> frameData) {
    while (tasks.size() > MAX_TASK) {
        // sleep 1ms
        LOGD("mpp_decoder_frame_callback waiting");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    {
        std::lock_guard<std::mutex> lock(mtx1);
        LOGD("Submit task %d", frameData->frameId);
        tasks.push(frameData);
        // tasks.push({id, img});
    }
    cv_task.notify_one();
    return NN_SUCCESS;
}

nn_error_e Yolov5ThreadPool::getTargetResult(std::vector<Detection> &objects, int id) {
    while (results.find(id) == results.end()) {
        // sleep 1ms
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::lock_guard<std::mutex> lock(mtx2);
    objects = results[id];
    // remove from map
    results.erase(id);
    img_results.erase(id);

    return NN_SUCCESS;
}

nn_error_e Yolov5ThreadPool::getTargetResultNonBlock(std::vector<Detection> &objects, int id) {
    if (results.find(id) == results.end()) {
        return NN_RESULT_NOT_READY;
    }
    std::lock_guard<std::mutex> lock(mtx2);
    objects = results[id];
    // remove from map
    results.erase(id);
    // img_results.erase(id);

    return NN_SUCCESS;
}

std::shared_ptr<frame_data_t> Yolov5ThreadPool::getTargetImgResult(int id) {
    int loop_cnt = 0;
    //    while (img_results.find(id) == img_results.end()) {
    //        // sleep 1ms
    //        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    //        loop_cnt++;
    //        if (loop_cnt > 1000) {
    //            NN_LOG_ERROR("getTargetImgResult timeout");
    //            return nullptr;
    //        }
    //    }
    std::lock_guard<std::mutex> lock(mtx2);
    auto frameData = img_results[id];
    // img = img_results[id];
    // remove from map
    // img_results.erase(id);
    return frameData;
    // results.erase(id);
    // return ;
}

// 停止所有线程
void Yolov5ThreadPool::stopAll() {
    stop = true;
    cv_task.notify_all();
}
