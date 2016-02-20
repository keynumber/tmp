/*
 * Author: number
 * Date  : Feb 8, 2016
 */

#ifndef __SERVER_ACCEPTOR_H_H___
#define __SERVER_ACCEPTOR_H_H___

#include <arpa/inet.h>      // for htons

#include <string>

#include "comm_struct.h"
#include "common/logger.h"

namespace ef {

class Poller;
class MessageCenter;

class Acceptor
{
    friend class MessageCenter;
public:
    Acceptor (unsigned short *port_arr, int len);
    virtual ~Acceptor ();

    bool Initialize(int id);
    void Run();
    void Stop();
    const std::string & GetErrMsg() const;

private:
    bool SetSocketOption(int fd);
    bool CreateListenSocket();

private:
    struct ListenerInfo {
        int fd = -1;
        unsigned short port = 0;
    };

private:
    int _acceptor_id;
    bool _run_flag;

    Poller *_poller;
    ListenerInfo *_listener;
    int _listener_len;

    std::string _errmsg;
};

} /* namespace ef */

#endif /* __SERVER_ACCEPTOR_H__ */
