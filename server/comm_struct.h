#ifndef __COMM_STRUCT_H__
#define __COMM_STRUCT_H__

#include <netinet/in.h>     // for sockaddr_in

#include <list>

#include "common/rc_buf.h"

namespace ef
{

enum FdType{
    kAcceptorToIohandlerQueue,
    kWorkerRspToIohandlerQueue,
    kClientFd,
};

struct FdInfo {
    int fd;
    uint32_t client_ip;
    uint16_t client_port;
    uint16_t type;
    timeval last_access_time;
    ef::RcBuf _to_request;       // 未满一个数据包,数据临时保存,真正存储数据的地方是Iohandler::_client_read_buf1/2中
    std::list<ef::RcBuf> _to_send;
};

struct AcceptInfo {
    int fd;
    sockaddr_in  addr;
};

// TODO: 可以用单独的通信queue来支持主流程外的其他通信
enum class NotifyCmd : unsigned char {
    // IoHandle and Worker
    ClientConnet,
    ClientDisconnet,
    WorkerRequestDisconnect,    // worker请求iohandler断开某个client的连接

    // Worker and Proxy
    WorkerRequestExtSvr,        // worker请求外部server
    ConnectExtSvrFailed,
    DisconnectExtSvr
};

struct IoHandlerReqToWorkerPack {
    int handler_id;
    FdInfo *fdinfo;
    ef::RcBuf request_buf;
};

struct WorkerRspToIoHandlerPack {
    int handler_id;
    FdInfo *fdinfo;
    ef::RcBuf response_buf;
};

} /* namespace ef */

#endif /* __COMM_STRUCT_H__ */
