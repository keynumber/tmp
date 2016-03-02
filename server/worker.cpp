/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#include "worker.h"

#include <string.h>

#include "net_complete_func.h"
#include "global_configure.h"
#include "common/rc_buf.h"
#include "common/debug.h"

extern ef::GlobalConfigure gGlobalConfigure;

namespace ef {

Worker::Worker()
    : _worker_id(0)
    , _run_flag(true)
    , _poller(gGlobalConfigure.worker_max_event_num)
    , _client_request_handler(nullptr)
    , _packet_header_len(header_len_func())
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

void Worker::RegistClientRequestHandler(void (*handler)(int handler_id, const ClientReqPack & req))
{
    _client_request_handler = handler;
}

void Worker::Run()
{
    LogKey("worker %d start to run\n", _worker_id);
    int event_num = 0;
    uint64_t key;
    uint32_t events;

    _run_flag = true;
    while (_run_flag) {
        event_num = _poller.Wait(100);
        if (unlikely(event_num < 0)) {
            LogErr("worker wait error, handler id: %d, errmsg: %s",
                    _worker_id, _poller.GetErrMsg().c_str());
            continue;
        }

        for (int i=0; i<event_num; ++i) {
            _poller.GetEvent(&key, &events);
            assert(key >= 0);

            ClientReqPack *p = _client_req_queue.Front();
            if (unlikely(!p)) {
                LogWarn("worker %d: was notified but no message\n", _worker_id);
                continue;
            }

            ClientReqPack req = *p;
            p->request_buf.Release();
            _client_req_queue.Take();
            _client_request_handler(_worker_id, req);
        }
    }

    LogKey("worker %d stop running\n", _worker_id);
}

void Worker::Stop()
{
    _run_flag = false;
}

const std::string & Worker::GetErrMsg() const
{
    return _errmsg;
}


} /* namespace ef */
