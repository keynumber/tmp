/*
 * Author: number
 * Date  : Nov 7, 2015
 */

#ifndef __COMMON_EVENT_NOTIFIER_H_H___
#define __COMMON_EVENT_NOTIFIER_H_H___

#include <stdint.h>

#include <string>

namespace ef {

/**
 * 仅仅用于事件通知,支持水平触发和边沿触发,即
 * 一次只消费一个事件,或者消费所有事件
 *
 * Note: 因为获取事件时,返回-1表示失败,因此返回int64_t,
 *       所以支持的最大的事件个数为LLONG_MAX
 *
 *
 * notice: EventNotifier 是线程安全的
 */
class EventNotifier {
public:
    EventNotifier();
    virtual ~EventNotifier();

    /**
     * @desc 设置task到来后的通知方式
     * @param blocked 是否已阻塞方式通知
     * @param edge_trigger 是否边沿触发方式通知
     */
    bool Initialize(bool blocked, bool edge_trigger);

    /**
     * @desc 通知事件,通知后,可以通过GetOneEvent/GetAllEvent获取事件个数
     * @param [in] num 通知事件的个数
     */
    bool Notify(uint64_t num);

    /**
     * @desc 获取事件的个数,当为边水平触发时
     *       一次消费一个通知,边沿通知,一次消费所有通知
     * @return 成功,返回事件个数;失败,返回-1
     */
    int64_t GetEvent();
    /**
     * @desc 获取通知用的文件描述符,一边外部通过epoll监控
     */
    int GetEventFd() const;

    const std::string & GetErrMsg() const;
private:
    int _fd;

    std::string _errmsg;
};

} /* namespace ef */

#endif /* __COMMON_EVENT_NOTIFIER_H__ */
