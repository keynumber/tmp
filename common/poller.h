/*
 * Author: number
 * Date  : Nov 1, 2015
 */

#ifndef __COMMON_POLLER_H_H___
#define __COMMON_POLLER_H_H___

#include <cstdint>

#include <string>

#include <sys/epoll.h>

#define DEFAULT_POLLER_MAX_EVENT_NUM 1024

namespace ef {

/**
 * Poller实现为非线程安全
 */
class Poller {
public:
    Poller(int max_event_num);
    virtual ~Poller();

    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    // 以下接口, 成功返回0, 失败返回-1
    int Add(int fd, uint64_t key, uint32_t events);
    int Modify(int fd, uint64_t key, uint32_t events);
    int Del(int fd);
    int GetEvent(uint64_t *key, uint32_t *events);
    // 成功返回待处理IO事件的文件描述符的个数,失败返回-1
    int Wait(int time_ms);
    const std::string & GetErrMsg() const;

private:
    int Ctrl(int fd, uint64_t key, int op, uint32_t events);

private:
    int _epoll_fd;
    epoll_event *_events;

    int _max_event_num;
    int _event_num;
    int _cur_index;

    std::string _errmsg;
};

} /* namespace ef */

#endif /* __COMMON_POLLER_H__ */
