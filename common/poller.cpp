/*
 * Author: number * Date  : Nov 1, 2015
 */

#include "poller.h"

#include <cassert>

#include "macro.h"
#include "wrapper.h"

namespace ef {

Poller::Poller(int max_event_num)
    : _max_event_num(max_event_num)
    , _event_num(0)
    , _cur_index(0)
{
    assert(max_event_num > 0);
    _epoll_fd = epoll_create(1);
    assert(_epoll_fd >= 0);
    _events =  new struct epoll_event[max_event_num];
    assert(_events != nullptr);
}

Poller::~Poller()
{
    delete []_events;
    safe_close(_epoll_fd);
}

int Poller::Add(int fd, uint64_t key, uint32_t events)
{
    return Ctrl(fd, key, EPOLL_CTL_ADD, events);
}

int Poller::Modify(int fd, uint64_t key, uint32_t events)
{
    return Ctrl(fd, key, EPOLL_CTL_MOD, events);
}

int Poller::Del(int fd)
{
    return Ctrl(fd, 0, EPOLL_CTL_DEL, 0);
}

int Poller::GetEvent(uint64_t *key, uint32_t *events)
{
    if (likely(_cur_index < _event_num)) {
        *key = _events[_cur_index].data.u64;
        *events = _events[_cur_index].events;
        ++_cur_index;
        return 0;
    }
    return -1;
}

int Poller::Wait(int time_ms)
{
    _cur_index = 0;
    do {
        _event_num = epoll_wait(_epoll_fd, &_events[0], _max_event_num, time_ms);
    } while (unlikely(_event_num < 0 && errno == EINTR));

    if (unlikely(_event_num < 0)) {
        _errmsg = safe_strerror(errno);
    }
    return _event_num;
}

int Poller::Ctrl(int fd, uint64_t key, int op, uint32_t events)
{
    struct epoll_event event;
    event.events = events;
    event.data.u64 = key;
    int ret = epoll_ctl(_epoll_fd, op, fd, &event);
    if (unlikely(ret < 0)) {
        _errmsg = safe_strerror(errno);
    }
    return ret;
}

const std::string & Poller::GetErrMsg() const
{
    return _errmsg;
}

} /* namespace ef */
