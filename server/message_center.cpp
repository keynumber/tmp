/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#include "message_center.h"

#include <assert.h>

#include "acceptor.h"
#include "io_handler.h"
#include "worker.h"
#include "common/macro.h"
#include "common/rc_buf.h"
#include "common/task_queue.h"
#include "common/wrapper.h"
#include "common/util.h"

namespace ef {

uint32_t MessageCenter::_last_iohandler_idx = 0;
uint32_t MessageCenter::_last_worker_idx = 0;

std::vector<Acceptor*> MessageCenter::_acceptors;
std::vector<IoHandler*> MessageCenter::_iohandlers;
std::vector<Worker*> MessageCenter::_workers;

MessageCenter::MessageCenter()
{
}

MessageCenter::~MessageCenter()
{
}

void MessageCenter::Register(Acceptor *acceptor)
{
    _acceptors.push_back(acceptor);
}

void MessageCenter::Register(IoHandler *iohandler)
{
    _iohandlers.push_back(iohandler);
}

void MessageCenter::Register(Worker *worker)
{
    _workers.push_back(worker);
}

void MessageCenter::PostAcceptClient(const AcceptInfo & accept_info)
{
    int size = _iohandlers.size();
    assert(size > 0);

    uint32_t idx = __sync_fetch_and_add(&_last_iohandler_idx, 1);
    idx = idx % size;
    if (unlikely(!_iohandlers[idx]->_accept_queue.Put(accept_info))) {
        safe_close(accept_info.fd);
        LogErr("iohandler %d accept queue is full, post accept client addr:%s:%d fd: %d failed,"
               "close client connection\n", idx,
               IpToString(accept_info.addr.sin_addr.s_addr).c_str(),
               ntohs(accept_info.addr.sin_port), accept_info.fd);
    }
}

void MessageCenter::PostClientReqToWorker(const ClientReqPack &req)
{
    int size = _workers.size();
    assert(size > 0);

    uint32_t idx = __sync_fetch_and_add(&_last_worker_idx, 1);
    idx = idx % size;

    // 如果投递失败,相当于丢请求,这种情况不处理
    _workers[idx]->_client_req_queue.Put(req);
}

void MessageCenter::PostSvrRspToClient(int iohandler_id, const ServerRspPack & rsp)
{
    // 如果投递失败,相当于丢请求,这种情况不处理
    _iohandlers[iohandler_id]->_worker_queue.Put(rsp);
}

} /* namespace ef */
