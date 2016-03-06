/*
 * Author: number
 * Date  : Mar 5, 2016
 */

#ifndef __COMMON_DATA_STRUCT_POOL_LIST_H_H___
#define __COMMON_DATA_STRUCT_POOL_LIST_H_H___

#include "fix_array.h"

namespace ef {

template <class T>
class PoolList {
public:
    PoolList() {}
    virtual ~PoolList() {}

    inline bool AttachPool(FixArray<T> * pool) { _pool = pool; return true; }
    inline T * PushBack() {
        int idx = _pool->Push(_tail);
        if (idx < 0) {
            return nullptr;
        }

        ++ _size;
        _tail = idx;
        if (_head < 0) {
            _head = _tail;
        }
        return &(*_pool)[_tail];
    }
    inline void PopFront() {
        if (_head >= 0) {
            --_size;
            int t = _pool->GetNext(_head);
            _pool->Pop(_head);
            _head = t;
            if (_head < 0) {
                _tail = _head;
            }
        }
    }
    inline void Clear() {
        _pool->PopList(_head);
        _head = _tail = -1;
        _size = 0;
    }
    inline int GetHead() const { return _head; }
    inline int GetTail() const { return _tail; }
    inline int GetNext(int idx) const { return _pool->GetNext(idx); }
    inline int Size() const { return _size; }
    inline T & operator[](int idx) { return (*_pool)[idx]; }

private:
    int _head = -1;
    int _tail = -1;
    int _size = 0;
    FixArray<T> * _pool = nullptr;
};

} /* namespace ef */

#endif /* __COMMON_DATA_STRUCT_POOL_LIST_H__ */
