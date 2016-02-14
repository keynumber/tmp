#ifndef __COMM_STRUCT_H__
#define __COMM_STRUCT_H__

#include <netinet/in.h>     // for sockaddr_in

struct AcceptInfo {
    int fd;
    sockaddr_in  addr;
};

struct ListenerInfo {
    unsigned short port = 0;
    int fd = -1;
};

#endif /* __COMM_STRUCT_H__ */
