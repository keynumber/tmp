/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#include "worker.h"

#include <string.h>

#include "global_configure.h"
#include "common/rc_buf.h"
#include "common/debug.h"

extern ef::GlobalConfigure gGlobalConfigure;

namespace ef {

Worker::Worker()
    : _worker_id(0)
    , _run_flag(true)
    , _poller(gGlobalConfigure.worker_max_event_num)
{
}

Worker::~Worker()
{
}

bool Worker::Initialize(int id)
{
    _worker_id = id;
    _run_flag = true;

    if (!_client_req_queue.Initialize(gGlobalConfigure.worker_request_queue_size, false)) {
        _errmsg = _client_req_queue.GetErrMsg();
        return false;
    }
    int notify_fd = _client_req_queue.GetNotifier();
    _poller.Add(notify_fd, notify_fd, EPOLLIN);
    return true;
}

void Worker::Run()
{
    LogKey("worker %d start to run\n", _worker_id);
    int event_num = 0;
    uint64_t key;
    uint32_t events;
    while (gGlobalConfigure.can_run && _run_flag) {
        event_num = _poller.Wait(100);
        if (unlikely(event_num < 0)) {
            LogErr("worker wait error, handler id: %d, errmsg: %s",
                    _worker_id, _poller.GetErrMsg().c_str());
            continue;
        }

        for (int i=0; i<event_num; ++i) {
            _poller.GetEvent(&key, &events);
            assert(key >= 0);

            TransferObj obj;
            if (unlikely(!_client_req_queue.Take(&obj))) {
                LogWarn("no message");
                continue;
            }
            HandleClientRequest(obj);
        }
    }
}

int Worker::HandleClientRequest(const TransferObj & obj)
{
    RcBuf rcbuf;
    rcbuf.FromTransferObj(obj);

    char buf[1024] = {0};
    memcpy(buf, rcbuf.buf + rcbuf.offset, rcbuf.len);
    for (int i = 0; i < rcbuf.len; ++i) {
        if(buf[i] == 0)
            buf[i] = '_';
    }
    DEBUG("worker %d: get request from iohandler buf offset %d, buf len %d\n",
          _worker_id, rcbuf.offset, rcbuf.len);
    DEBUG("request buf: |%s|\n", buf);

    return 0;
}

void Worker::Stop()
{
    LogKey("worker %d stop running\n", _worker_id);
    _run_flag = false;
}

const std::string & Worker::GetErrMsg() const
{
    return _errmsg;
}


} /* namespace ef */
