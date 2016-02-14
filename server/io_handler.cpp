/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#include "io_handler.h"

#include <assert.h>
#include <string.h>

#include "global_configure.h"
#include "message_center.h"
#include "default_net_complete_func.h"
#include "common/macro.h"
#include "common/debug.h"
#include "common/util.h"
#include "common/wrapper.h"

extern ef::GlobalConfigure gGlobalConfigure;

namespace ef {

IoHandler::IoHandler()
    : _handler_id(0)
    , _run_flag(true)
    , _poller(gGlobalConfigure.iohandler_max_event_num)
    , _fd_array(gGlobalConfigure.iohandler_fd_array_size)
    , _fd_expire_queue(gGlobalConfigure.iohandler_fd_array_size)
    , _client_read_buf1(new RcBuf(gGlobalConfigure.iohandler_read_buf_len))
    , _client_read_buf2(new RcBuf(gGlobalConfigure.iohandler_read_buf_len))
    , _net_complete_func(default_net_complete_func)
    , _minimum_packet_len_func(default_minimum_packet_len_func)
    , _minimum_packet_len(_minimum_packet_len_func())
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
        _errmsg = _accept_queue.GetErrMsg();
        return false;
    }
    int accept_queue_fd = _accept_queue.GetNotifier();

    int accept_queue_idx = _fd_array.Push(FdInfo());
    _fd_array[accept_queue_idx].fd = accept_queue_fd;
    _fd_array[accept_queue_idx].type = kAcceptQueue;
    _poller.Add(accept_queue_fd, accept_queue_idx, EPOLLIN);
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
            case kAcceptQueue:
            {
                AcceptInfo accinfo;
                if (unlikely(!_accept_queue.Take(&accinfo))) {
                    LogWarn("iohandle %d: has notified but no message\n", _handler_id);
                    continue;
                }
                HandleAcceptClient(accinfo);
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
    char * unfulfiled_buf = _fd_array[idx].rc_buf.buf + _fd_array[idx].rc_buf.offset;
    uint32_t unfulfiled_len = _fd_array[idx].rc_buf.len;
    if (unfulfiled_len > buf1->len) {
        DELETE_POINTER(buf1);
        buf1 = buf2;
        buf2 = new RcBuf(gGlobalConfigure.iohandler_read_buf_len);
    }
    memcpy(buf1->buf + buf1->len, unfulfiled_buf, unfulfiled_len);
    // buf1->offset += unfulfiled_len;
    // buf1->len -= unfulfiled_len;

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
    uint32_t handle_len = HandleClientBuf(buf1, remain_data_len);
    if (unlikely(handle_len < 0)) {
        LogInfo("iohandler %d: client %s:%d fd %d request data check failed with net_complete_func, close connection",
                _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
        CloseClientConn(idx);
        return false;
    }
    // TODO buf1 use all used , need to alloc new rcbuf

    // 数据跨越两个buffer, 并且够计算包大小
    remain_data_len -= handle_len;
    if (remain_data_len > buf1->len) {
        // 将数据拷贝到tmpbuf中计算下一个包多大
        bool has_theory_len = true;
        uint32_t theory_len;

        if (remain_data_len > _minimum_packet_len) {
            char tmpbuf[_minimum_packet_len];
            if (buf1->len > _minimum_packet_len) {
                memcpy(tmpbuf, buf1->buf + buf1->offset, _minimum_packet_len);
            } else {
                memcpy(tmpbuf, buf1->buf + buf1->offset, buf1->len);
                memcpy(tmpbuf+buf1->len, buf2->buf+buf2->offset, _minimum_packet_len-buf1->len);
            }

            int ret = _net_complete_func(tmpbuf, _minimum_packet_len, &theory_len);
            if (unlikely(ret < 0)) {
                LogInfo("iohandler %d: client %s:%d fd %d request data check failed with net_complete_func, close connection",
                        _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
                CloseClientConn(idx);
                return false;
            }
        } else {
            has_theory_len = false;
        }

        // buf1和buf2中的数据能够构成一个完整的包,申请额外的空间构成一个连续的buf存放跨buf1和buf2的那个请求
        if (remain_data_len >= theory_len + _minimum_packet_len) {
            RcBuf rcbuf(theory_len);
            // 将完整的数据包拷贝到rcbuf中
            {
                uint32_t copy_len = 0;
                uint32_t skip_len = _minimum_packet_len;

                uint32_t skip_buf1_len = buf1->len > skip_len ? skip_len : buf1->len ;
                buf1->len -= skip_buf1_len;
                buf1->offset += skip_buf1_len;
                copy_len = buf1->len;
                memcpy(rcbuf.buf, buf1->buf+buf1->offset, copy_len);
                buf1->len -= copy_len;
                buf1->offset += copy_len;
                rcbuf.offset += copy_len;

                skip_len -= skip_buf1_len;
                copy_len = theory_len - copy_len;

                int skip_buf2_len = buf2->len > skip_len ? skip_len : buf2->len ;
                buf2->len -= skip_buf2_len;
                buf2->offset += skip_buf2_len;
                memcpy(rcbuf.buf + rcbuf.offset, buf2->buf+buf2->offset, copy_len);
                buf2->offset += copy_len;
                buf2->len -= copy_len;
                rcbuf.offset += copy_len;

                remain_data_len -= (theory_len + _minimum_packet_len);
            }

            TransferObj obj;
            rcbuf.ToTransferObj(&obj, 0, theory_len);
            MessageCenter::PostClientReqToWorker(obj);

            // 释放已经使用完的buf1
            DELETE_POINTER(buf1);
            buf1 = buf2;
            buf2 = new RcBuf(gGlobalConfigure.iohandler_read_buf_len);

            handle_len = HandleClientBuf(buf1, remain_data_len);
            if (unlikely(handle_len < 0)) {
                LogInfo("iohandler %d: client %s:%d fd %d request data check failed with net_complete_func, close connection",
                        _handler_id, IpToString(fdinfo.client_ip).c_str(), fdinfo.client_port, fdinfo.fd);
                CloseClientConn(idx);
                return false;
            }
            remain_data_len -= handle_len;
        }

        // buf1和buf2中的数据不能够构成一个完整的包,则将buf1中的数据拷贝到buf2中
        if (!has_theory_len) {
            uint32_t data_len_in_buf2 = remain_data_len - buf1->len;
            char buf[gGlobalConfigure.max_packet_size];
            memcpy(buf, buf1->buf+buf1->offset, buf1->len);
            memcpy(buf+buf1->len, buf2->buf+buf2->offset, remain_data_len-buf1->len);
            memcpy(buf2->buf, buf, remain_data_len);
            buf2->offset = remain_data_len;
            buf2->len = remain_data_len;

            // 释放已经使用完的buf1
            DELETE_POINTER(buf1);
            buf1 = buf2;
            buf2 = new RcBuf(gGlobalConfigure.iohandler_read_buf_len);
            return true;
        }
    }

    if (remain_data_len > 0) {
        _fd_array[idx].rc_buf.Copy(*buf1, buf1->offset, remain_data_len);
        buf1->offset += remain_data_len;
        buf1->len -= remain_data_len;
    }

    return true;
}

int IoHandler::HandleClientBuf(RcBuf *rcbuf, uint32_t len)
{
    uint32_t remain_data_len = len > rcbuf->len ? rcbuf->len : len;
    uint32_t handle_len = 0;
    uint32_t theory_packet_len = 0;

    TransferObj obj;
    while (remain_data_len >= _minimum_packet_len) {
        int ret = _net_complete_func(rcbuf->buf + rcbuf->offset, remain_data_len, &theory_packet_len);
        if (likely(ret > 0)) {
            if (ret + _minimum_packet_len > rcbuf->len) {   // 不够一个数据包
                break;
            }

            rcbuf->ToTransferObj(&obj, rcbuf->offset + _minimum_packet_len, ret);
            MessageCenter::PostClientReqToWorker(obj);

            ret += _minimum_packet_len;
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
    if (_poller.Del(fd) < 0) {
        const FdInfo & fdinfo = _fd_array[idx];
        LogErr("iohandler %d: delete fd %d failed, cient %s:%d, errmsg: %s\n",
               _handler_id, fdinfo.fd, IpToString(fdinfo.client_ip).c_str(),
               ntohs(fdinfo.client_port), _poller.GetErrMsg().c_str());
    }
    _fd_array.Pop(idx);
    _fd_expire_queue.Erase(idx);
    safe_close(fd);
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
