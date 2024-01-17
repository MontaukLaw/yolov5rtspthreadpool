#include "display_queue.h"

void RenderFrameQueue::push(std::shared_ptr<frame_data_t> &frameDataPtr) {
    // 获取队列的长度
    if (m_queue.size() > DISPLAY_QUEUE_MAX_SIZE) {
        LOGD("RenderFrameQueue::push queue size > 60");
        delete frameDataPtr->data;
        frameDataPtr->data = nullptr;
        return;
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(frameDataPtr);
    lock.unlock();
    m_cond.notify_one();
}

std::shared_ptr<frame_data_t> RenderFrameQueue::pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this] { return !m_queue.empty(); });
    auto data = m_queue.front();
    m_queue.pop();
    return data;
}

int RenderFrameQueue::size() {
    return m_queue.size();
}

