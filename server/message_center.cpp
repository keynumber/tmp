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

void MessageCenter::PostAcceptClient(int fd, const sockaddr_in &addr)
{
    int size = _iohandlers.size();
    assert(size > 0);

    AcceptInfo accept_info;
    accept_info.fd = fd;
    accept_info.addr = addr;

    uint32_t idx = __sync_fetch_and_add(&_last_iohandler_idx, 1);
    idx = idx % size;
    _iohandlers[idx]->_accept_queue.Put(accept_info);
}

void MessageCenter::PostClientReqToWorker(const TransferObj &obj)
{
    int size = _workers.size();
    assert(size > 0);

    uint32_t idx = __sync_fetch_and_add(&_last_worker_idx, 1);
    idx = idx % size;
    _workers[idx]->_client_req_queue.Put(obj);
}

} /* namespace ef */