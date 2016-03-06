/*
 * Author: number
 * Date  : Nov 8, 2015
 */

#ifndef __COMMON_DATA_STRUCT_FIX_QUEUE_H_H___
#define __COMMON_DATA_STRUCT_FIX_QUEUE_H_H___

#include <assert.h>

#include "common/macro.h"

namespace ef {

/**
 * 定长的队列,长度满后插入失败
 * 通过两个变量分别记录队列的队首和队尾:
 * _head: 下一个位置是可读的位置
 * _tail: 当前位置是可写的位置
 * 因此,判空条件为: _head = (_head + 1) % _max_size;
 *        满条件为:(_tail + 1) % _max_size == _head;
 * 因此需要两个额外的对象,因为,当队列总长度为2时,队列始终为空,也始终为满
 */
template<class T>
class FixQueue {
public:
    FixQueue(int size)
    {
        _max_size = size + 2;
        _queue = new T[_max_size];
        assert(_queue != nullptr);
        _head = 0;
        _tail = 1;
    }

    virtual ~FixQueue()
    {
        assert(_queue != nullptr);
        DELETE_ARRAY(_queue);
    }

    FixQueue(const FixQueue & q) = delete;
    FixQueue & operator = (const FixQueue & q) = delete;
    FixQueue(const FixQueue && q) = delete;
    FixQueue & operator = (const FixQueue && q) = delete;

    inline bool push(const T & v)
    {
        if (!full()) {
            _queue[_tail] = v;
            _tail = (_tail + 1) % _max_size;
            return true;
        }
        return false;
    }

    inline void pop()
    {
        if (!empty()) {
            int idx = (_head + 1) % _max_size;
            // _queue[idx].~T();
            // 调用对象的析构函数,可能会释放对象的部分资源,当重新利用该对象时,即对
            // 该对象重新赋值时,则会出现SEGMENT FAULT
            // 例如对象是,std::list,调用析构函数时,如果list中包含对象,析构函数会
            // 调用list的析构函数,释放list中存在的对象,不过list貌似没有把内部指针
            // 置null,当重新利用该对象时,赋值时会导致先释放原有list中的对象,此时
            // list内部包含野指针,进程就直接挂了
            _head = idx;
        }
    }

    // 有个小小的问题, 队列为空,front返回的对象是无效的,不过现在只有taskqueue在用,事件驱动是没有问题的
    inline T * front() const
    {
        if (!empty()) {
            return &_queue[(_head + 1) % _max_size];
        }
        return nullptr;
    }

    inline T * back() const
    {
        if (!empty()) {
            return *_queue[(_tail - 1 + _max_size) % _max_size];
        }
        return nullptr;
    }

    inline bool size() const
    {
        return _max_size - 1;
    }

    inline bool empty() const
    {
        return (_head + 1) % _max_size == _tail;
    }

    inline bool full() const
    {
        return (_tail + 1) % _max_size == _head;
    }

private:
    T * _queue;
    int _max_size;
    int _head;
    int _tail;
};

} /* namespace ef */

#endif /* __COMMON_DATA_STRUCT_FIX_QUEUE_H__ */
