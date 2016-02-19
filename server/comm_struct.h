#ifndef __COMM_STRUCT_H__
#define __COMM_STRUCT_H__

#include <netinet/in.h>     // for sockaddr_in

#include <list>

#include "common/rc_buf.h"

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

struct IoHandlerReqToWorkerPack {
    FdInfo *fdinfo;
    ef::RcBuf request_buf;
};

struct WorkerRspToIoHandlerPack {
    FdInfo *fdinfo;
    ef::RcBuf response_buf;
};

#endif /* __COMM_STRUCT_H__ */
