/*
 * Author: number
 * Date  : Feb 10, 2016
 */

#ifndef __COMMON_DATA_STRUCT_EXPIRE_QUEUE_H_H___
#define __COMMON_DATA_STRUCT_EXPIRE_QUEUE_H_H___

#include <stdint.h>

#include "common/macro.h"

namespace ef {

class ExpireQueue {
public:
    ExpireQueue(uint32_t capacity);
    virtual ~ExpireQueue();

    void Activate(int idx);
    void Erase(int idx);

    inline uint32_t Capacity() const { return _capacity; }
    inline int GetHead() const { return _queue_next[_capacity]; }
    inline int GetTail() const { return _capacity == _tail ? -1 : _tail; }
    inline int GetNext(int idx) const { return _queue_next[idx]; }
    inline int GetPre(int idx) const {
        int pre = _queue_pre[idx];
        if (unlikely(pre == _capacity)) {
            pre = -1;
        }
        return pre;
    }

private:
    int *_queue_pre;
    int *_queue_next;
    uint32_t _capacity;

    int _tail;
};

} /* namespace ef */

#endif /* __COMMON_DATA_STRUCT_EXPIRE_QUEUE_H__ */
