/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#include "io_handler.h"

#include <assert.h>
#include <string.h>

#include "global_configure.h"
#include "message_center.h"
#include "common/macro.h"
#include "common/debug.h"
#include "common/util.h"
#include "common/wrapper.h"

extern ef::GlobalConfigure gGlobalConfigure;

namespace ef {

IoHandler::IoHandler()
    : _handler_id(0)
    , _run_flag(true)
    , _fd_array(gGlobalConfigure.iohandler_fd_array_size)
    , _fd_expire_queue(gGlobalConfigure.iohandler_fd_array_size)
    , _client_read_buf1(new RcBuf(gGlobalConfigure.iohandler_read_buf_len))
    , _client_read_buf2(new RcBuf(gGlobalConfigure.iohandler_read_buf_len))
    , _poller(gGlobalConfigure.iohandler_max_event_num)
    , _rcbuf_pool(gGlobalConfigure.iohandler_socket_buf_pool_size)
    , _packet_len_func(packet_len_func)
    , _header_len_func(header_len_func)
    , _header_len(_header_len_func())
{
    assert(_client_read_buf1);
    assert(_client_read_buf2);

    for (int i=0; i<gGlobalConfigure.iohandler_fd_array_size; i++) {
        _fd_array[i]._to_request.AttachPool(&_rcbuf_pool);
        _fd_array[i]._to_send.AttachPool(&_rcbuf_pool);
    }
}

IoHandler::~IoHandler()
{
    Stop();

    DELETE_POINTER(_client_read_buf1);
    DELETE_POINTER(_client_read_buf2);
}

bool IoHandler::Initialize(int handler_id)
{
    _handler_id = handler_id;
    _run_flag = true;

    if (!_accept_queue.Initialize(gGlobalConfigure.iohandler_accept_queue_size, false)) {
        _errmsg = std::string("iohandler queue acceptor to iohandler initialize failed, ")
                + _accept_queue.GetErrMsg();
        return false;
    }
    int accept_queue_fd = _accept_queue.GetNotifier();
    uint64_t accept_queue_idx = static_cast<uint64_t>(_fd_array.Push());
    _fd_array[accept_queue_idx].fd = accept_queue_fd;
    _fd_array[accept_queue_idx].type = kAcceptorToIohandlerQueue;
    _poller.Add(accept_queue_fd, accept_queue_idx, EPOLLIN);

    if (!_worker_queue.Initialize(gGlobalConfigure.iohandler_worker_rsp_queue_size, false)) {
        _errmsg = std::string("iohandler queue worker response to iohandler initialize failed, ")
                + _worker_queue.GetErrMsg();
        return false;
    }
    int worker_queue_fd = _worker_queue.GetNotifier();
    uint64_t worker_queue_idx = static_cast<uint64_t>(_fd_array.Push());
    _fd_array[worker_queue_idx].fd = worker_queue_fd;
    _fd_array[worker_queue_idx].type = kWorkerRspToIohandlerQueue;
    _poller.Add(worker_queue_fd, worker_queue_idx, EPOLLIN);
    return true;
}

void IoHandler::Run()
{
    LogKey("iohandler %d start to run\n", _handler_id);
    gettimeofday(&_now, nullptr);

    int event_num;
    uint64_t key;
    uint32_t events;

    _run_flag = true;
    while (_run_flag) {
        event_num = _poller.Wait(100);
        if (unlikely(event_num < 0)) {
            LogErr("iohandler %d: wait error, errmsg: %s", _handler_id, _poller.GetErrMsg().c_str());
            continue;
        }

        while (_poller.GetEvent(&key, &events) == 0) {
            FdInfo & fdinfo = _fd_array[key];
            switch (fdinfo.type) {
            case kAcceptorToIohandlerQueue:
            {
                AcceptInfo *p = _accept_queue.Front();
                if (unlikely(!p)) {
                    LogWarn("iohandle %d: was notified but no message\n", _handler_id);
                    continue;
                }
                AcceptInfo accinfo = *p;
                _accept_queue.Take();
                HandleAcceptClient(accinfo);
                break;
            }
            case kWorkerRspToIohandlerQueue:
            {
                ServerRspPack *p = _worker_queue.Front();
                if (unlikely(!p)) {
                    LogWarn("iohandle %d: worker was notified but no response message\n", _handler_id);
                    continue;
                }

                ServerRspPack rsp = *p;
                p->response_buf.Release();
                _worker_queue.Take();
                HandleWorkerRsp(rsp);               // 写请求不改变fd最后活跃时间
                break;
            }
            case kClientFd:
            {
                if (likely(events & EPOLLIN)) {
                    HandleClientRequest(key);       // 读请求会改变最后活跃时间
                } else if (events & EPOLLOUT){
                    SendDataToClient(fdinfo, true);       // 写请求不改变fd最后活跃时间
                }

                break;
            }
            default:
            {
                // TODO wrap assert to log and really exit, for release edition
                LogErr("iohandler %d: unreachable branch\n", _handler_id);
                assert(false);
                break;
            }
            }
        }

        gettimeofday(&_now, nullptr);
        TimeTick();
    }

    LogKey("iohandler %d stop running\n", _handler_id);
}

bool IoHandler::HandleAcceptClient(const AcceptInfo & accinfo)
{
    LogFrame("iohandler %d: accept client from %s:%d, fd: %d\n", _handler_id,
            IpToString(static_cast<int>(accinfo.addr.sin_addr.s_addr)).c_str(),
            ntohs(accinfo.addr.sin_port), accinfo.fd);
    if (unlikely(set_nonblock(accinfo.fd) < 0)) {
        LogErr("iohandler %d: accept client from %s:%d, fd: %d set_nonblock failed\n",
                 _handler_id, IpToString(static_cast<int>(accinfo.addr.sin_addr.s_addr)).c_str(),
                 ntohs(accinfo.addr.sin_port), accinfo.fd);
        safe_close(accinfo.fd);
        return false;
    }

    uint64_t client_idx= static_cast<uint64_t>(_fd_array.Push());
    _fd_array[client_idx].fd = accinfo.fd;
    _fd_array[client_idx].idx = client_idx;
    _fd_array[client_idx].client_ip = static_cast<int>(accinfo.addr.sin_addr.s_addr);
    _fd_array[client_idx].client_port = accinfo.addr.sin_port;
    _fd_array[client_idx].type = kClientFd;
    _fd_array[client_idx].last_access_time = _now;

    _fd_expire_queue.Activate(client_idx);
    _poller.Add(accinfo.fd, client_idx, EPOLLIN);
    return true;
}

void IoHandler::TimeTick()
{
    int next = _fd_expire_queue.GetHead();
    while (next >= 0) {
        int idx = next;
        next = _fd_expire_queue.GetNext(next);

        int gap = (_now.tv_sec - _fd_array[idx].last_access_time.tv_sec) * 1000 +
                  (_now.tv_usec - _fd_array[idx].last_access_time.tv_usec) / 1000;
        if (gap > gGlobalConfigure.client_max_inaction_time_ms) {
            LogDebug("iohandler %d: client %s:%u is inactive more than %dms, server disconnect\n",
                     _handler_id, IpToString(_fd_array[idx].client_ip).c_str(), _fd_array[idx].client_port,
                     gGlobalConfigure.client_max_inaction_time_ms);
            CloseClientConn(idx);
            continue;
        }
        break;
    }
}

int IoHandler::CopyUncompleteDataToBuf1(FdInfo & fdinfo) {
    RcBuf * & buf1 = _client_read_buf1;
    RcBuf * & buf2 = _client_read_buf2;

    PoolList<RcBuf> & to_request = fdinfo._to_request;

    // 计算收缓冲区总大小
    int uncomplete_len = 0;
    int idx = to_request.GetHead();
    while (idx > 0) {
        uncomplete_len += to_request[idx].len;
        idx = to_request.GetNext(idx);
    }

    // 如果buf1内存不够,重新申请
    if (buf1->len <= uncomplete_len) {
        DELETE_POINTER(buf1);
        buf1 = buf2;
        buf2 = new RcBuf(gGlobalConfigure.iohandler_read_buf_len);
    }

    // 拷贝收缓冲区数据到读buf,并释放收缓冲区内容
    char * buf = buf1->buf + buf2->offset;
    idx = to_request.GetHead();
    while (idx > 0) {
        RcBuf & t = to_request[idx];
        memcpy(buf, t.buf + t.offset, t.len);
        t.Release();
        idx = to_request.GetNext(idx);
    }
    to_request.Clear();
    return uncomplete_len;
}

// 请求发送到worker后,继续处理client的发送数据时出错, 导致关闭fd,
// 然后另外的连接使用了这个对象,导致worker响应数据错发,
// 因此,这块改造了FixArray的Push策略,优先使用未用过的对象,而不是刚刚释放的对象
// 不过也应该尽量避免这类问题,导致关闭client连接的因素有
//     1. 读socket出错(不可控)
//     2. client数据出错(不可控
//     3. fd申请socket buffer时,buffer已经用完,导致出错,可控,因此应该尽可能的避免这个问题)
bool IoHandler::HandleClientRequest(int idx)
{
    FdInfo & fdinfo = _fd_array[idx];
    fdinfo.last_access_time = _now;

    // 拷贝上次未构成完整包的数据到读buffer
    int uncomplete_len = CopyUncompleteDataToBuf1(fdinfo);
    int remain_data_len = uncomplete_len;
    RcBuf * & buf1 = _client_read_buf1;
    RcBuf * & buf2 = _client_read_buf2;

    // 从客户端读取数据
    {
        struct iovec vec[2];
        vec[0].iov_base = buf1->buf + buf1->offset + uncomplete_len;
        vec[0].iov_len = static_cast<size_t>(buf1->len - uncomplete_len);
        vec[1].iov_base = buf2->buf + buf2->offset;
        vec[1].iov_len = static_cast<size_t>(buf2->len);

        int len = safe_readv(fdinfo.fd, vec, 2);
        if (unlikely(len < 0)) {
            _errmsg = safe_strerror(errno);
            LogErr("iohandler %d: read from client %s:%d fd %d error, close connection, errmsg: %s\n",
                   _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd, _errmsg.c_str());
            CloseClientConn(idx);
            return false;
        } else if (len == 0) {
            LogFrame("iohandler %d: client %s:%d fd %d close disconnection\n", _handler_id,
                     IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
            CloseClientConn(idx);
            return true;
        }

        remain_data_len += len;

        DEBUG("iohandler %d: read from client %s:%d fd %d data len %d byts\n",
              _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd, len);
    }

    // 处理buf1中读到的数据
    {
        int handle_len = HandleClientBuf(idx, buf1, remain_data_len);
        if (unlikely(handle_len < 0)) {
            LogErr("iohandler %d: client %s:%d fd %d request data check failed with packet_len_func, close connection",
                   _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
            CloseClientConn(idx);
            return false;
        }

        remain_data_len -= handle_len;
    }

    // 有数据剩余
    if (remain_data_len > 0) {
        // 数据没有跨buf,直接将数据保存即可
        if (remain_data_len <= buf1->len) {
            RcBuf * b = fdinfo._to_request.PushBack();
            if (b) {
                b->Copy(*buf1, buf1->offset, remain_data_len);
                buf1->offset += remain_data_len;
                buf1->len -= remain_data_len;
            } else {
                LogErr("iohandler %d: client %s:%d fd %d alloc socket rcbuf failed, close connection",
                        _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
                CloseClientConn(idx);
            }
        }
        // 处理可能的跨buf1和buf2的数据
        else {
            // 能够计算包长
            if (remain_data_len >= _header_len) {
                char * p = nullptr;
                char tmpbuf[_header_len];
                if (buf1->len >= _header_len) {
                    p = buf1->buf + buf1->offset;
                } else {
                    p = tmpbuf;
                    memcpy(tmpbuf, buf1->buf + buf1->offset, static_cast<size_t>(buf1->len));
                    memcpy(tmpbuf+buf1->len, buf2->buf+buf2->offset, static_cast<size_t>(_header_len-buf1->len));
                }

                int ret = _packet_len_func(p, _header_len);
                if (unlikely(ret < 0)) {
                    LogErr("iohandler %d: client %s:%d fd %d request data check failed with packet_len_func, close connection",
                            _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
                    CloseClientConn(idx);
                    return false;
                }

                // 够一个完整数据包的长度
                if (remain_data_len >= ret) {
                    RcBuf tmpbuf(ret);
                    // buf1中包含部分数据
                    int buf1_copy_len = buf1->len;
                    int buf2_copy_len = ret - buf1->len;
                    memcpy(tmpbuf.buf, buf1->buf + buf1->offset, buf1_copy_len);
                    memcpy(tmpbuf.buf + buf1_copy_len, buf2->buf + buf2->offset, buf2_copy_len);

                    buf1->offset += buf1_copy_len;
                    buf1->len = 0;
                    buf2->offset += buf2_copy_len;
                    buf2->len -= buf2_copy_len;

                    remain_data_len -= ret;

                    ClientReqPack req;
                    req.fdinfo = &fdinfo;
                    req.handler_id = _handler_id;
                    req.request_buf = tmpbuf;
                    MessageCenter::PostClientReqToWorker(req);

                    int handle_len = HandleClientBuf(idx, buf2, remain_data_len);
                    if (unlikely(handle_len < 0)) {
                        LogErr("iohandler %d: client %s:%d fd %d request data check failed with packet_len_func, close connection",
                                _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
                        CloseClientConn(idx);
                        return false;
                    }

                    remain_data_len -= handle_len;

                    // 处理完buf2,还有数据剩余
                    if (remain_data_len > 0) {
                        RcBuf * b = fdinfo._to_request.PushBack();
                        if (b) {
                            b->Copy(*buf2, buf2->offset, remain_data_len);
                            buf2->offset += remain_data_len;
                            buf2->len -= remain_data_len;
                        } else {
                            LogErr("iohandler %d: client %s:%d fd %d alloc socket rcbuf failed, close connection",
                                    _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
                            CloseClientConn(idx);
                        }
                    }
                } else {
                    // 数据跨buf,但不够一个完整数据包的长度
                    RcBuf * b1 = fdinfo._to_request.PushBack();
                    RcBuf * b2 = fdinfo._to_request.PushBack();
                    if (b1 && b2) {
                        int buf1_remain_len = buf1->len;
                        int buf2_remain_len = remain_data_len - buf1_remain_len;
                        b1->Copy(*buf1, buf1->offset, buf1_remain_len);
                        b2->Copy(*buf2, buf2->offset, buf2_remain_len);
                        buf1->len -= buf1_remain_len;
                        buf1->offset += buf1_remain_len;
                        buf2->len -= buf2_remain_len;
                        buf2->offset += buf2_remain_len;
                    } else {
                        LogErr("iohandler %d: client %s:%d fd %d alloc socket rcbuf failed, close connection",
                                _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
                        CloseClientConn(idx);
                    }
                }
            } else {
                // 数据跨buf,但不够计算数据包长度
                RcBuf * b1 = fdinfo._to_request.PushBack();
                RcBuf * b2 = fdinfo._to_request.PushBack();
                if (b1 && b2) {
                    int buf1_remain_len = buf1->len;
                    int buf2_remain_len = remain_data_len - buf1_remain_len;
                    b1->Copy(*buf1, buf1->offset, buf1_remain_len);
                    b2->Copy(*buf2, buf2->offset, buf2_remain_len);
                    buf1->len -= buf1_remain_len;
                    buf1->offset += buf1_remain_len;
                    buf2->len -= buf2_remain_len;
                    buf2->offset += buf2_remain_len;
                } else {
                    LogErr("iohandler %d: client %s:%d fd %d alloc socket rcbuf failed, close connection",
                            _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
                    CloseClientConn(idx);
                }
            }
        }
    }

    // 如果buf1/buf2用完了,重新申请
    if (buf1->len <= 0) {
        DELETE_POINTER(buf1);
        buf1 = buf2;
        buf2 = new RcBuf(gGlobalConfigure.iohandler_read_buf_len);
        assert(buf2);
        if (buf1->len == 0) {
            DELETE_POINTER(buf1);
            buf1 = buf2;
            buf2 = new RcBuf(gGlobalConfigure.iohandler_read_buf_len);
            assert(buf2);
        }
    }
    return true;
}

int IoHandler::HandleClientBuf(int idx, RcBuf *rcbuf, int len)
{
    int remain_data_len = len > rcbuf->len ? rcbuf->len : len;
    int handle_len = 0;

    ClientReqPack req;
    req.fdinfo = &_fd_array[idx];
    req.handler_id = _handler_id;
    while (remain_data_len >= _header_len) {
        int ret = _packet_len_func(rcbuf->buf + rcbuf->offset, remain_data_len);
        if (unlikely(ret < 0)){
            return -1;
        }

        if (ret > 0) {
            if (ret > remain_data_len) {   // 不够一个数据包
                break;
            }

            req.request_buf.Copy(*rcbuf, rcbuf->offset, ret);
            MessageCenter::PostClientReqToWorker(req);

            rcbuf->offset += ret;
            rcbuf->len -= ret;
            handle_len += ret;
            remain_data_len -= ret;
        } else {
            break;
        }
    }
    return handle_len;
}

    static int socket_num = 0;

void IoHandler::CloseClientConn(int idx)
{
    ++socket_num;
    FdInfo & fdinfo = _fd_array[idx];
    int fd = fdinfo.fd;
    // 必须先从poller中删除fd,才能关闭socket fd,否则,反过来的话,fd先被关闭
    // 则poller删除fd时,会报错bad file descriptor,并且该fd会被poller持续触发,
    // 导致iohandler cpu占用持续100%
    // safe_close(fd);
    _poller.Del(fd);
    // _fd_array.Pop支持析构函数
    // 会自动释放掉fd关联的用户请求数据和待发送数据,而不需要显示调用进行析构
    _fd_array.Pop(idx);
    _fd_expire_queue.Erase(idx);
    safe_close(fd);

    // 释放fd关联收发缓冲区的RcBuf
    PoolList<RcBuf> & request_list = fdinfo._to_request;
    int t = request_list.GetHead();
    while (t >= 0) {
        request_list[t].Release();
        t = request_list.GetNext(t);
    }
    request_list.Clear();

    PoolList<RcBuf> & send_list = fdinfo._to_request;
    t = send_list.GetHead();
    while (t >= 0) {
        send_list[t].Release();
        t = send_list.GetNext(t);
    }
    send_list.Clear();

    // 防止worker响应前,socket被关闭,发错数据到其他文件描述符
    fdinfo.fd = -1;
}

bool IoHandler::HandleWorkerRsp(const ServerRspPack & rsp)
{
    FdInfo & fdinfo = *rsp.fdinfo;
    RcBuf * buf = fdinfo._to_send.PushBack();       // 添加到末尾
    if (unlikely(nullptr == buf)) {
        LogErr("iohandler %d: client %s:%d fd %d alloc socket rcbuf failed, close connection",
               _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
        CloseClientConn(fdinfo.idx);
        return false;
    }

    *buf = rsp.response_buf;
    return SendDataToClient(fdinfo, false);
}

bool IoHandler::SendDataToClient(FdInfo & fdinfo, bool is_epoll_out)
{
    const int max_iovec_count = 64;
    struct iovec towrite[max_iovec_count];

    int total_need_send = 0;
    int cnt = 0;
    PoolList<RcBuf> & send_list = fdinfo._to_send;
    int t = send_list.GetHead();
    while (t >= 0 && cnt < max_iovec_count) {
        towrite[cnt].iov_base = _rcbuf_pool[t].buf + _rcbuf_pool[t].offset;
        towrite[cnt].iov_len = _rcbuf_pool[t].len;
        total_need_send += _rcbuf_pool[t].len;
        t = send_list.GetNext(t);
        ++cnt;
    }

    if (t >= 0) {
        LogErr("iohandler %d: client %s:%d fd %d to send buf is too large, close connection",
               _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
        CloseClientConn(fdinfo.idx);
        return false;
    }

    int fd = fdinfo.fd;
    int write_bytes = safe_writev(fd, towrite, cnt);
    if (unlikely( write_bytes < 0)) {
        LogErr("iohandler %d: write fd %d failed, cient %s:%d, errmsg: %s\n",
               _handler_id, fdinfo.fd, IpToString(fdinfo.client_ip).c_str(),
               ntohs(fdinfo.client_port), safe_strerror(errno).c_str());
        CloseClientConn(fdinfo.idx);
        return false;
    }

    int tmp = write_bytes;
    t = send_list.GetHead();
    while (tmp > 0) {
        if (tmp >= send_list[t].len) {
            tmp -= send_list[t].len;

            send_list[t].Release();
            send_list.PopFront();
            tmp = send_list.GetHead();
        } else {
            send_list[t].len -= tmp;
            send_list[t].offset += tmp;
            break;
        }
    }

    if (unlikely(write_bytes < total_need_send)) {
        _poller.Modify(fd, static_cast<uint64_t>(fdinfo.idx), EPOLLOUT | EPOLLIN | EPOLLHUP | EPOLLERR);
        DEBUG("iohandler %d: write fd %d cient %s:%d %d bytes, need total write %d bytes,add EPOLLOUT\n",
               _handler_id, fd, IpToString(static_cast<int>(fdinfo.client_ip)).c_str(),
               ntohs(fdinfo.client_port),
               write_bytes, total_need_send);
    } else {
        if (unlikely(is_epoll_out)) {
            // 发送数据完毕,去掉EPOLLOUT
            _poller.Modify(fd, static_cast<uint64_t>(fdinfo.idx), EPOLLIN| EPOLLHUP | EPOLLERR);
        }
    }
    return true;
}

void IoHandler::Stop()
{
    _run_flag = false;
}

const std::string & IoHandler::GetErrMsg() const
{
    return _errmsg;
}

} /* namespace ef */
