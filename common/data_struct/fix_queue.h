/*
 * Author: number
 * Date  : Nov 8, 2015
 */

#ifndef __COMMON_DATA_STRUCT_FIX_QUEUE_H_H___
#define __COMMON_DATA_STRUCT_FIX_QUEUE_H_H___

#include <assert.h>
#include <stdint.h>

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
    FixQueue(uint32_t size)
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
        delete [] _queue;
    }

    FixQueue(const FixQueue & q) = delete;
    FixQueue & operator = (const FixQueue & q) = delete;
    FixQueue(const FixQueue && q) = delete;
    FixQueue & operator = (const FixQueue && q) = delete;

    bool push(T v)
    {
        if (!full()) {
            _queue[_tail] = v;
            _tail = (_tail + 1) % _max_size;
            return true;
        }
        return false;
    }

    void pop()
    {
        if (!empty()) {
            _head = (_head + 1) % _max_size;
        }
    }

    const T & front() const
    {
        return _queue[(_head + 1) % _max_size];
    }

    const T & back() const
    {
        return _queue[(_tail - 1 + _max_size) % _max_size];
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
    uint32_t _max_size;
    uint32_t _head;
    uint32_t _tail;
};

} /* namespace ef */

#endif /* __COMMON_DATA_STRUCT_FIX_QUEUE_H__ */
