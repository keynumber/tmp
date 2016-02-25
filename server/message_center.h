/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#ifndef __SERVER_MESSAGE_CENTER_H_H___
#define __SERVER_MESSAGE_CENTER_H_H___

#include <vector>

#include "comm_struct.h"

namespace ef {

class Acceptor;
class IoHandler;
class Worker;
struct TransferObj;

class MessageCenter {
public:
    static void Register(Acceptor *acceptor);
    static void Register(IoHandler *iohandler);
    static void Register(Worker *worker);

    static void PostAcceptClient(const AcceptInfo & accept_info);
    static void PostClientReqToWorker(const IoHandlerReqToWorkerPack & req);
    static void PostSvrRspToClient(int iohandler_id, const WorkerRspToIoHandlerPack & rsp);

private:
    MessageCenter();
    virtual ~MessageCenter();

private:
    static std::vector<Acceptor*> _acceptors;
    static std::vector<IoHandler*> _iohandlers;
    static std::vector<Worker*> _workers;

    static uint32_t _last_iohandler_idx;
    static uint32_t _last_worker_idx;
};

} /* namespace ef */

#endif /* __SERVER_MESSAGE_CENTER_H__ */
