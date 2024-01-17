#ifndef AIBOX_DISPLAY_QUEUE_H
#define AIBOX_DISPLAY_QUEUE_H

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>
#include "log4c.h"
#include "user_comm.h"

#define DISPLAY_QUEUE_MAX_SIZE 60
using namespace std;

class RenderFrameQueue {
public:
    void push(std::shared_ptr <frame_data_t> &frameDataPtr);

    std::shared_ptr <frame_data_t> pop();

    int size();

private:
    queue <std::shared_ptr<frame_data_t>> m_queue;
    mutex m_mutex;
    condition_variable m_cond;
};

#endif //AIBOX_DISPLAY_QUEUE_H
