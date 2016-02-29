/*
 * Author: number
 * Date  : Feb 8, 2016
 */

#include "acceptor.h"

#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "common/util.h"
#include "comm_struct.h"
#include "global_configure.h"
#include "message_center.h"
#include "common/wrapper.h"
#include "common/poller.h"
#include "common/util.h"

extern ef::GlobalConfigure gGlobalConfigure;

namespace ef {

Acceptor::Acceptor(unsigned short *port_arr, int len)
    : _acceptor_id(0)
    , _run_flag(true)
    , _poller(new Poller(gGlobalConfigure.acceptor_max_event_num))
    , _listener(new ListenerInfo[len])
    , _listener_len(len)
{
    assert(_poller);
    assert(_listener);
    for (int i=0; i<len; ++i) {
        _listener[i].port = port_arr[i];
    }
}

Acceptor::~Acceptor()
{
    Stop();

    if (_listener) {
        for (int i=0; i<_listener_len; ++i) {
            if (_listener[i].fd >= 0) {
                safe_close(_listener[i].fd);
                LogKey("close listen socket on port %d\n", _listener[i].port);
            }
        }

        delete [] _listener;
        _listener = nullptr;
        _listener_len = 0;
    }

    if (_poller) {
        delete _poller;
        _poller = nullptr;
    }
}

bool Acceptor::CreateListenSocket()
{
    for (int i=0; i<_listener_len; ++i) {
        // TODO socket param
        LogKey("try to listen on port %d\n", _listener[i].port);
        _listener[i].fd = socket(AF_INET, SOCK_STREAM, 0);
        if (_listener[i].fd < 0) {
            _errmsg = "socket() error, " + safe_strerror(errno);
            return false;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_listener[i].port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (!SetSocketOption(_listener[i].fd)) {
            return false;
        }

        if (bind(_listener[i].fd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0) {
            _errmsg = "bind() error, " + safe_strerror(errno);
            return false;
        }

        // TODO listen param
        if (listen(_listener[i].fd, 1000) < 0) {
            _errmsg = "listen() error, " + safe_strerror(errno);
            return false;
        } else {
            // TODO write log
            LogKey("listen on port %d success\n", _listener[i].port);
        }
    }

    return true;
}

bool Acceptor::Initialize(int id)
{
    _acceptor_id = id;
    _run_flag = true;

    if (!CreateListenSocket()) {
        return false;
    }

    for (int i=0; i<_listener_len; ++i) {
        if (_poller->Add(_listener[i].fd, i, EPOLLIN | EPOLLERR | EPOLLHUP) < 0) {       // TODO 只监听EPOLLIN么?其他的事件呢?
            _errmsg = "error happend when add listen fd to poller, " + _poller->GetErrMsg();
            LogErr("error happend when add listen fd to poller, errmsg: %s\n",
                    _poller->GetErrMsg().c_str());
            return false;
        }
    }
    return true;
}

void Acceptor::Run()
{
    LogKey("acceptor %d start to run\n", _acceptor_id);
    int ret = 0;

    _run_flag = true;
    while (_run_flag) {
        ret = _poller->Wait(100);  // block TODO how many milsec to wait
        if (unlikely(ret < 0)) {
            LogErr("error happend when poller wait, errmsg: %s\n",
                    _poller->GetErrMsg().c_str());
            continue;
        }

        uint64_t key = -1;
        uint32_t events = 0;
        for (int i=0; i<ret; ++i) {
            _poller->GetEvent(&key, &events);
            if (likely(events & EPOLLIN)) {
                sockaddr_in  addr;
                socklen_t addr_len = sizeof(sockaddr_in);
                int fd = accept(_listener[key].fd, (sockaddr*)&addr, &addr_len);
                if (unlikely(fd < 0)) {
                    LogErr("accept socket on port %d failed, errmsg: %s\n",
                            _listener[key].port, safe_strerror(errno).c_str());
                    continue;
                }

                LogFrame("accept client from %s:%d, fd: %d\n",
                        IpToString(addr.sin_addr.s_addr).c_str(), ntohs(addr.sin_port), fd);

                AcceptInfo accept_info;
                accept_info.fd = fd;
                accept_info.addr = addr;
                MessageCenter::PostAcceptClient(accept_info);
            } else {
                LogErr("listen socket on port %d get event not EPOLLIN, but %d\n",
                        _listener[key].port, events);
            }
        }
    }

    LogKey("acceptor %d stop running\n", _acceptor_id);
}

void Acceptor::Stop()
{
    _run_flag = false;
}

bool Acceptor::SetSocketOption(int fd)
{
    // TODO
    // int option = 1;
    // setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char*)&option,sizeof(option));
    if (set_nonblock(fd) < 0) {
        _errmsg = "set_nonblock() error, " + safe_strerror(errno);
    }
    return true;
}

const std::string & Acceptor::GetErrMsg() const
{
    return _errmsg;
}

} /* namespace ef */
