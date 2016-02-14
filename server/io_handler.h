/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#ifndef __SERVER_IO_HANDLER_H_H___
#define __SERVER_IO_HANDLER_H_H___

#include <string>

#include <sys/time.h>

#include "comm_struct.h"
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

private:
    enum FdType{
        kAcceptQueue,
        kClientFd,
    };

    struct FdInfo {
        int fd;
        uint32_t client_ip;
        uint16_t client_port;
        uint16_t type;
        timeval last_access_time;
        RcBuf rc_buf;       // 未满一个数据包,数据临时保存,真正存储数据的地方是Iohandler::_client_read_buf1/2中
    };

private:
    bool HandleAcceptClient(const AcceptInfo & accinfo);
    bool HandleClientRequest(int idx);
    // < 0 出错
    // > 0 返回处理数据的长度
    int HandleClientBuf(RcBuf *rcbuf, uint32_t len);

    void CloseClientConn(int idx);

private:
    int _handler_id;
    bool _run_flag;

    timeval _now;

    Poller _poller;

    FixArray<FdInfo> _fd_array;
    ExpireQueue _fd_expire_queue;
    RcBuf *_client_read_buf1;
    RcBuf *_client_read_buf2;

    // acceptor接收到的连接传递到iohandler
    TaskQueue<AcceptInfo> _accept_queue;

    // from worker rsp buffer
    // socket write buffer


    // buf 为数据内容指针
    // len 为数据长度
    // theory_len 完整包应该的长度
    // 返回 > 0, 表示一个完整包的长度
    // 返回 = 0, 表示包不完整,如果包长大于最小数据包的长度,则theory_len为理论包长
    // 返回 < 0, 数据出错,不符合包规范
    typedef int (*net_complete_func)(char *buf, uint32_t len, uint32_t * theoy_len);
    // 最小数据包的长度,根据这个长度的数据就能知道包大小
    typedef int (*minimum_packet_len_func)();
    net_complete_func _net_complete_func;
    minimum_packet_len_func _minimum_packet_len_func ;

    const uint32_t _minimum_packet_len;

    std::string _errmsg;
};

} /* namespace ef */

#endif /* __SERVER_IO_HANDLER_H__ */
