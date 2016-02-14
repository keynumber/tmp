/*
 * Author: number
 * Date  : Nov 7, 2015
 */

#include "event_notifier.h"

#include <unistd.h>
#include <sys/eventfd.h>

#include "wrapper.h"
#include "macro.h"

namespace ef {

EventNotifier::EventNotifier() : _fd(-1)
{
}

EventNotifier::~EventNotifier()
{
    if (_fd >= 0) {
        safe_close(_fd);
    }
}

bool EventNotifier::Initialize(bool blocked, bool edge_trigger)
{
    int flags = EFD_CLOEXEC;
    if (!blocked) {
        flags = flags | EFD_NONBLOCK;
    }

    if (!edge_trigger) {
        flags = flags | EFD_SEMAPHORE;
    }

    _fd = eventfd(0, flags);
    if (unlikely(_fd < 0)) {
        _errmsg = safe_strerror(errno);
        return false;
    }
    return true;
}

bool EventNotifier::Notify(uint64_t num)
{
    ssize_t ret = safe_write(_fd, &num, sizeof(uint64_t));
    if (likely(ret == sizeof(uint64_t))) {
        return true;
    }

    _errmsg = safe_strerror(errno);
    return ret == sizeof(uint64_t);
}

int64_t EventNotifier::GetEvent()
{
    uint64_t num = 0;
    ssize_t ret = safe_read(_fd, &num, sizeof(uint64_t));
    if (likely(ret == sizeof(uint64_t))) {
        return static_cast<int64_t>(num);
    }

    _errmsg = safe_strerror(errno);
    return -1;
}

int EventNotifier::GetEventFd() const
{
    return _fd;
}

const std::string & EventNotifier::GetErrMsg() const
{
    return _errmsg;
}

} /* namespace ef */
