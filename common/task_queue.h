/*
 * Author: number
 * Date  : Nov 8, 2015
 */

#ifndef __COMMON_TASK_QUEUE_H_H___
#define __COMMON_TASK_QUEUE_H_H___

#include <stdint.h>
#include <assert.h>

#include <mutex>

#include "event_notifier.h"
#include "data_struct/fix_queue.h"
#include "macro.h"

namespace ef {

class EventNotifier;
template<class T> class FixQueue;

/**
 * @desc 线程安全,固定长度的任务池,采用eventfd进行通知
 */
template <class T>
class TaskQueue {
public:
    TaskQueue()
    {
        _queue = nullptr;
        _notifier = nullptr;
    }

    virtual ~TaskQueue()
    {
        DELETE_POINTER(_notifier);
        DELETE_POINTER(_queue);
    }

    // TODO
    TaskQueue(const TaskQueue &) = delete;
    TaskQueue(const TaskQueue &&) = delete;
    TaskQueue & operator = (const TaskQueue &) = delete;
    TaskQueue & operator = (const TaskQueue &&) = delete;

    /**
     * @desc 设置task到来后的通知方式
     * @param blocked 是否已阻塞方式通知
     */
    bool Initialize(int size, bool blocked)
    {
        _queue = new FixQueue<T>(size);
        assert(_queue != nullptr);
        _notifier = new EventNotifier();
        assert(_notifier != nullptr);
        bool ret = _notifier->Initialize(blocked, false);
        if (!ret) {
            _errmsg = _notifier->GetErrMsg();
        }
        return ret;
    }

    /**
     * @desc 将任务放入队列,并通知
     */
    inline bool Put(const T & task)
    {
        _lock.lock();
        bool ret = _queue->push(task);
        _lock.unlock();

        if (ret) {
            _notifier->Notify(1);
        }
        return ret;
    }

    inline T * Front()
    {
        return _queue->front();
    }

    /**
     * @desc 将获得的任务放入*task
     * @return 成功,返回true,失败返回false
     */
    inline bool Take()
    {
        bool ret = false;
        _lock.lock();
        if (!_queue->empty()) {
            _queue->pop();
            ret = true;
        }
        _lock.unlock();

        if (ret) {
            _notifier->GetEvent();
        }
        return ret;
    }

    inline int GetNotifier() const { return _notifier->GetEventFd(); }
    inline const std::string & GetErrMsg() const { return _errmsg; }

private:
    FixQueue<T> *_queue;
    std::mutex _lock;
    EventNotifier * _notifier;

    std::string _errmsg;
};

} /* namespace ef */

#endif /* __COMMON_TASK_QUEUE_H__ */
