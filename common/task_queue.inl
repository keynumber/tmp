/*
 * Author: number
 * Date  : Nov 8, 2015
 */

#include <assert.h>

#include "event_notifier.h"
#include "data_struct/fix_queue.h"
#include "macro.h"

namespace ef {

template <class T>
TaskQueue<T>::TaskQueue()
{
}

template <class T>
TaskQueue<T>::~TaskQueue()
{
    DELETE_POINTER(_notifier);
    DELETE_POINTER(_queue);
}

template <class T>
bool TaskQueue<T>::Initialize(uint32_t size, bool blocked)
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

template <class T>
bool TaskQueue<T>::Put(const T & task)
{
    _lock.lock();
    bool ret = _queue->push(task);
    _lock.unlock();

    if (ret) {
        _notifier->Notify(1);
    }
    return ret;
}

template <class T>
bool TaskQueue<T>::Take(T *task)
{
    bool ret = false;
    _lock.lock();
    if (!_queue->empty()) {
        *task = _queue->front();
        _queue->pop();
        ret = true;
    }
    _lock.unlock();

    _notifier->GetEvent();
    return ret;
}

template <class T>
int TaskQueue<T>::GetNotifier() const
{
    return _notifier->GetEventFd();
}

template <class T>
const std::string & TaskQueue<T>::GetErrMsg() const
{
    return _errmsg;
}

} /* namespace ef */
