/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#ifndef __SERVER_WORKER_H_H___
#define __SERVER_WORKER_H_H___

#include <string>

#include "common/task_queue.h"
#include "common/poller.h"

namespace ef {

class MessageCenter;
class TransferObj;

class Worker {
    friend class MessageCenter;
public:
    Worker();
    virtual ~Worker();

    bool Initialize(int id);
    void Run();
    void Stop();
    const std::string & GetErrMsg() const;

private:
    int HandleClientRequest(const TransferObj & obj);
private:
    int _worker_id;
    bool _run_flag;

    Poller _poller;

    // acceptor接收到的连接传递到iohandler
    TaskQueue<TransferObj> _client_req_queue;

    std::string _errmsg;
};

} /* namespace ef */

#endif /* __SERVER_WORKER_H__ */
