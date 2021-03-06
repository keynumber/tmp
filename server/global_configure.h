/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#ifndef __SERVER_GLOBAL_CONFIGURE_H_H___
#define __SERVER_GLOBAL_CONFIGURE_H_H___

#include "common/logger.h"

namespace ef
{

struct GlobalConfigure {
    int client_max_inaction_time_ms = 50000;

    LogLevel log_level = kLevelFrame;

    int acceptor_count = 1;
    int iohandler_count = 1;
    int woker_count = 1;

    int acceptor_max_event_num = 1024;         // max poller event

    /* 设置的大了可以提高性能,但也不能太高,这样会影响iohandler的定时任务 */
    int max_accept_client_per_cycle = 1024;

    int iohandler_max_event_num = 1024;        // max poller event
    int iohandler_accept_queue_size = 20480;   // io handler accept queue size
    int iohandler_fd_array_size = 20480;       // io handler fd array size
    int iohandler_read_buf_len = 4*1024*1024;  // buffer的长度至少为请求包的最大数据长度,否则设计的临时buffer不能缓存所有的数据,就会出错
    int iohandler_worker_rsp_queue_size = 10240;
    int iohandler_socket_buf_pool_size = 20480;    // 没有实际的占用内存,只是socket最多有多少各buf

    int worker_max_event_num = 1024;         // max poller event
    int worker_request_queue_size = 20480;    // io handler accept queue size
};

} /* namespace ef */

#endif /* __SERVER_GLOBAL_CONFIGURE_H__ */
