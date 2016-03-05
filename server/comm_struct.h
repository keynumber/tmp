#ifndef __COMM_STRUCT_H__
#define __COMM_STRUCT_H__

#include <netinet/in.h>     // for sockaddr_in

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
    int idx;               // 在iohandler进行管理的_fd_array中的下标
    int client_ip;
    uint16_t client_port;
    uint16_t type;
    timeval last_access_time;
    int _to_request_idx = -1;         // 未满一个数据包,数据临时保存,真正存储数据的地方是Iohandler::_client_read_buf1/2中
    int _to_send_head = -1;           // 可能多个响应都没发出去,全积压在iohandler,但是收包最多只会存一个不完整的包
    int _to_send_tail = -1;
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

struct ClientReqPack {
    int handler_id;
    FdInfo *fdinfo;
    ef::RcBuf request_buf;
};

struct ServerRspPack {
    int handler_id;
    FdInfo *fdinfo;
    ef::RcBuf response_buf;
};

} /* namespace ef */

#endif /* __COMM_STRUCT_H__ */
