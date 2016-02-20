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
    , _packet_len_func(packet_len_func)
    , _header_len_func(header_len_func)
    , _header_len(_header_len_func())
{
    assert(_client_read_buf1);
    assert(_client_read_buf2);
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
    int accept_queue_idx = _fd_array.Push(FdInfo());
    _fd_array[accept_queue_idx].fd = accept_queue_fd;
    _fd_array[accept_queue_idx].type = kAcceptorToIohandlerQueue;
    _poller.Add(accept_queue_fd, accept_queue_idx, EPOLLIN);

    if (!_worker_queue.Initialize(gGlobalConfigure.iohandler_worker_rsp_queue_size, false)) {
        _errmsg = std::string("iohandler queue worker response to iohandler initialize failed, ")
                + _worker_queue.GetErrMsg();
        return false;
    }
    int worker_queue_fd = _worker_queue.GetNotifier();
    int worker_queue_idx = _fd_array.Push(FdInfo());
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

    while (gGlobalConfigure.can_run && _run_flag) {
        event_num = _poller.Wait(100);
        if (unlikely(event_num < 0)) {
            LogErr("iohandler %d: wait error, errmsg: %s",
                    _handler_id, _poller.GetErrMsg().c_str());
            continue;
        }

        for (int i=0; i<event_num; ++i) {
            _poller.GetEvent(&key, &events);
            assert(key >= 0);
            if (unlikely(!(events & EPOLLIN))) {
                LogErr("iohandler %d: get event not EPOLLIN, but %d, fd index: %d\n",
                        _handler_id, events, (uint32_t)key);
                continue;
            }

            const FdInfo & fdinfo = _fd_array[key];
            switch (fdinfo.type) {
            case kAcceptorToIohandlerQueue:
            {
                AcceptInfo accinfo;
                if (unlikely(!_accept_queue.Take(&accinfo))) {
                    LogWarn("iohandle %d: has notified but no message\n", _handler_id);
                    continue;
                }
                HandleAcceptClient(accinfo);
                break;
            }
            case kWorkerRspToIohandlerQueue:
            {
                WorkerRspToIoHandlerPack rsp;
                if (unlikely(!_worker_queue.Take(&rsp))) {
                    LogWarn("iohandle %d: worker has notified but no response message\n", _handler_id);
                    continue;
                }
                HandleWorkerRsp(rsp);
                break;
            }
            case kClientFd:
            {
                HandleClientRequest(key);
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
}

bool IoHandler::HandleAcceptClient(const AcceptInfo & accinfo)
{
    LogFrame("iohandler %d: accept client from %s:%d, fd: %d\n", _handler_id,
            IpToString(accinfo.addr.sin_addr.s_addr).c_str(), ntohs(accinfo.addr.sin_port), accinfo.fd);
    if (unlikely(set_nonblock(accinfo.fd) < 0)) {
        LogFrame("iohandler %d: accept client from %s:%d, fd: %d set_nonblock failed\n",
                 _handler_id, IpToString(accinfo.addr.sin_addr.s_addr).c_str(),
                 ntohs(accinfo.addr.sin_port), accinfo.fd);
        safe_close(accinfo.fd);
        return false;
    }

    int client_idx= _fd_array.Push(FdInfo());
    _fd_array[client_idx].fd = accinfo.fd;
    _fd_array[client_idx].client_ip = accinfo.addr.sin_addr.s_addr;
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

bool IoHandler::HandleClientRequest(int idx)
{
    FdInfo & fdinfo = _fd_array[idx];
    fdinfo.last_access_time = _now;

    RcBuf * & buf1 = _client_read_buf1;
    RcBuf * & buf2 = _client_read_buf2;

    // copy 上次未构成完整包的数据
    int unfulfiled_len = _fd_array[idx]._to_request.len;
    if (_fd_array[idx]._to_request.buf) {
        char * unfulfiled_buf = _fd_array[idx]._to_request.buf + _fd_array[idx]._to_request.offset;
        if (unfulfiled_len > buf1->len) {
            DELETE_POINTER(buf1);
            buf1 = buf2;
            buf2 = new RcBuf(gGlobalConfigure.iohandler_read_buf_len);
        }
        memcpy(buf1->buf + buf1->offset, unfulfiled_buf, unfulfiled_len);
        _fd_array[idx]._to_request.Release();
    }

    DEBUG("iohandler %d: last from client %s:%d fd %d unfulfiled data len %d byts\n",
          _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd, unfulfiled_len);


    struct iovec vec[2];
    vec[0].iov_base = buf1->buf + buf1->offset + unfulfiled_len;
    vec[0].iov_len = buf1->len - unfulfiled_len;
    vec[1].iov_base = buf2->buf + buf2->offset;
    vec[1].iov_len = buf2->len;

    int len = safe_readv(fdinfo.fd, vec, 2);
    if (unlikely(len < 0)) {
        // TODO port need ntol
        _errmsg = safe_strerror(errno);
        LogInfo("iohandler %d: read from client %s:%d fd %d error, close connection\n",
                _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
        CloseClientConn(idx);
        return false;
    } else if (len == 0) {
        LogInfo("iohandler %d: client %s:%d fd %d close disconnection\n", _handler_id,
                IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
        CloseClientConn(idx);
        return true;
    }

    DEBUG("iohandler %d: read from client %s:%d fd %d data len %d byts\n",
            _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd, len);

    int remain_data_len = len + unfulfiled_len;
    int handle_len = HandleClientBuf(idx, buf1, remain_data_len);
    if (unlikely(handle_len < 0)) {
        LogInfo("iohandler %d: client %s:%d fd %d request data check failed with packet_len_func, close connection",
                _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
        CloseClientConn(idx);
        return false;
    }

    remain_data_len -= handle_len;

    // 剩余读到的buf长度大于最小包长
    int next_packet_remain_data = 0;
    int next_packet_theory_len = 0;
    if (remain_data_len >= _header_len) {
        char * p = nullptr;
        char tmpbuf[_header_len];
        if (buf1->len >= _header_len) {
            p = buf1->buf + buf1->offset;
        } else {
            p = tmpbuf;
            memcpy(tmpbuf, buf1->buf + buf1->offset, buf1->len);
            memcpy(tmpbuf+buf1->len, buf2->buf+buf2->offset, _header_len-buf1->len);
        }

        int ret = _packet_len_func(p, _header_len, &next_packet_theory_len);
        if (unlikely(ret < 0)) {
            LogInfo("iohandler %d: client %s:%d fd %d request data check failed with packet_len_func, close connection",
                    _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
            CloseClientConn(idx);
            return false;
        }
        if (remain_data_len >= next_packet_theory_len + _header_len) {
            next_packet_remain_data = next_packet_theory_len + _header_len;
        } else {
            next_packet_remain_data = remain_data_len;
        }
    } else {
        next_packet_remain_data = remain_data_len;
    }

    // 如果存在数句跨buf,则进行数据拷贝
    if (next_packet_remain_data > buf1->len) {
        // 当数据全在buf2中时,也会进行数据拷贝,怎么处理?????????????
        RcBuf tmpbuf(next_packet_remain_data);
        // buf1中包含部分数据
        memcpy(tmpbuf.buf, buf1->buf + buf1->offset, buf1->len);
        // 后边拷贝还需要用到buf1->len
        // buf1->offset += buf1->len;
        // buf1->len = 0;
        int tmp2_copy_len = next_packet_remain_data - buf1->len;
        memcpy(tmpbuf.buf + buf1->len, buf2->buf + buf2->offset, next_packet_remain_data - buf1->len);
        buf1->offset += buf1->len;
        buf1->len = 0;
        buf2->offset += tmp2_copy_len;
        buf2->len -= tmp2_copy_len;

        remain_data_len -= next_packet_remain_data;

        // 如果临时buf中的数据够一个完整的包,即能够计算出包长,并且包长大于等于
        // 这时候buf1中的数据肯定用完,并且buf2中肯定包含数据
        // (因为HandleClientBuf会处理,直到不能构成一个完整的包)
        if (next_packet_remain_data >= _header_len &&
            next_packet_remain_data >= _header_len + next_packet_theory_len) {
            IoHandlerReqToWorkerPack req;
            tmpbuf.offset += _header_len;    // 跳过包头
            tmpbuf.len -= _header_len;    // 跳过包头
            req.fdinfo = &_fd_array[idx];
            req.request_buf = tmpbuf;
            MessageCenter::PostClientReqToWorker(req);

            handle_len = HandleClientBuf(idx, buf2, remain_data_len);
            if (unlikely(handle_len < 0)) {
                LogInfo("iohandler %d: client %s:%d fd %d request data check failed with packet_len_func, close connection",
                        _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
                CloseClientConn(idx);
                return false;
            }

            // buf2中有为处理完的数据
            remain_data_len -= handle_len;
            if (remain_data_len > 0) {
                fdinfo._to_request.Copy(*buf2, buf2->offset, remain_data_len);
                buf2->offset += remain_data_len;
                buf2->len -= remain_data_len;
            }
        } else {
            // 如果临时数据中的不能构成一个包,则直接放在fd的缓存buf中就行了
            fdinfo._to_request = tmpbuf;
        }
    } else if (next_packet_remain_data > 0) {
        fdinfo._to_request.Copy(*buf1, buf1->offset, next_packet_remain_data);
        buf1->offset += next_packet_remain_data;
        buf1->len -= next_packet_remain_data;
    }

    // 如果buf1/buf2用完了,重新申请
    if (buf1->len == 0) {
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
    int theory_packet_len = 0;

    IoHandlerReqToWorkerPack req;
    req.fdinfo = &_fd_array[idx];
    while (remain_data_len >= _header_len) {
        int ret = _packet_len_func(rcbuf->buf + rcbuf->offset, remain_data_len, &theory_packet_len);
        if (likely(ret > 0)) {
            if (ret + _header_len > remain_data_len) {   // 不够一个数据包
                break;
            }

            req.request_buf.Copy(*rcbuf, rcbuf->offset + _header_len, theory_packet_len);
            MessageCenter::PostClientReqToWorker(req);

            ret += _header_len;
            rcbuf->offset += ret;
            rcbuf->len -= ret;
            handle_len += ret;
            remain_data_len -= ret;

        } else if (unlikely(ret < 0)){
            return -1;
        } else {
            break;
        }
    }
    return handle_len;
}

void IoHandler::CloseClientConn(int idx)
{
    int fd = _fd_array[idx].fd;
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
}

bool IoHandler::HandleWorkerRsp(const WorkerRspToIoHandlerPack & rsp)
{
    FdInfo & fdinfo = *rsp.fdinfo;
    fdinfo._to_send.push_back(rsp.response_buf);
    return SendDataToClient(fdinfo);
    // TODO epollout when data is not all sent
}

bool IoHandler::SendDataToClient(FdInfo & fdinfo)
{
    const int max_iovec_count = 10 * 1024;
    int size = fdinfo._to_send.size();
    if (unlikely(size > max_iovec_count)) {
        LogErr("iohandler %d: _to_send buf is too large close client fd %d, cient %s:%d\n",
               _handler_id, fdinfo.fd, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port);
        return false;
    }

    int cnt = 0;
    struct iovec towrite[max_iovec_count];
    for (const auto & it : fdinfo._to_send) {
        towrite[cnt].iov_base = it.buf + it.offset;
        towrite[cnt].iov_len = it.len;
        ++cnt;
    }

    int fd = fdinfo.fd;
    int ret = safe_writev(fd, towrite, cnt);
    if (unlikely( ret < 0)) {
        LogErr("iohandler %d: write fd %d failed, cient %s:%d, errmsg: %s\n",
               _handler_id, fdinfo.fd, IpToString(fdinfo.client_ip).c_str(),
               ntohs(fdinfo.client_port), safe_strerror(errno).c_str());
        return false;
    }

    auto it = fdinfo._to_send.begin();
    auto next = it;
    while (ret > 0) {
        it = next;
        next = ++it;
        if (ret >= it->len) {
            ret -= it->len;
            fdinfo._to_send.erase(it);
        } else {
            it->len -= ret;
            it->offset += ret;
            break;
        }
    }
    return true;
}

void IoHandler::Stop()
{
    LogKey("iohandler %d stop running\n", _handler_id);
    _run_flag = false;
}

const std::string & IoHandler::GetErrMsg() const
{
    return _errmsg;
}

} /* namespace ef */
