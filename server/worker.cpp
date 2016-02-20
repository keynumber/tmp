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

            IoHandlerReqToWorkerPack req;
            if (unlikely(!_client_req_queue.Take(&req))) {
                LogWarn("no message");
                continue;
            }
            HandleClientRequest(req);
        }
    }

    LogKey("worker %d stop running\n", _worker_id);
}

int Worker::HandleClientRequest(const IoHandlerReqToWorkerPack &req)
{
    const RcBuf & rcbuf = req.request_buf;

    PacketHeader * header = (PacketHeader*)(rcbuf.buf + rcbuf.offset);
    char * buf = rcbuf.buf + rcbuf.offset + sizeof(PacketHeader);

    for (int i = 0; i<header->length; ++i) {
        if(buf[i] == 0)
            buf[i] = '_';
    }

    DEBUG("worker %d: get request from iohandler buf offset %d, buf len %d, content len: %d, request id: %d\n",
          _worker_id, rcbuf.offset, rcbuf.len, header->length, header->request_id);
    DEBUG("request buf: |%s|\n", buf);

    return 0;
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
