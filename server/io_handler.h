/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#ifndef __SERVER_IO_HANDLER_H_H___
#define __SERVER_IO_HANDLER_H_H___

#include <string>

#include <sys/time.h>

#include "comm_struct.h"
#include "net_complete_func.h"
#include "common/rc_buf.h"
#include "common/poller.h"
#include "common/task_queue.h"
#include "common/data_struct/expire_queue.h"
#include "common/data_struct/fix_array.h"

namespace ef {

class IoHandler {
    friend class MessageCenter;
public:
    IoHandler();
    virtual ~IoHandler();

    bool Initialize(int id);
    void Run();
    void Stop();
    void TimeTick();
    const std::string & GetErrMsg() const;
    // inline const FdInfo & GetFdInfo(int idx) const { return _fd_array[idx]; }

private:
    bool HandleAcceptClient(const AcceptInfo & accinfo);
    bool HandleClientRequest(int idx);
    bool HandleWorkerRsp(const ServerRspPack & rsp);
    bool SendDataToClient(FdInfo & fdinfo);
    // < 0 出错
    // > 0 返回处理数据的长度
    int HandleClientBuf(int idx, RcBuf *rcbuf, int len);

    void CloseClientConn(int idx);

private:
    int _handler_id;
    bool _run_flag;
    timeval _now;

    FixArray<FdInfo> _fd_array;
    ExpireQueue _fd_expire_queue;
    RcBuf *_client_read_buf1;
    RcBuf *_client_read_buf2;

    Poller _poller;
    // acceptor接收到的连接传递到iohandler
    TaskQueue<AcceptInfo> _accept_queue;
    // worker发送响应数据到iohandler
    TaskQueue<ServerRspPack> _worker_queue;

    FixArray<RcBuf> _rcbuf_pool;

    ppacket_len_func _packet_len_func;
    pheader_len_func _header_len_func ;
    const int _header_len;

    std::string _errmsg;
};

} /* namespace ef */

#endif /* __SERVER_IO_HANDLER_H__ */
