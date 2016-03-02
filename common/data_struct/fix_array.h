/*
 * Author: number
 * Date  : Feb 11, 2016
 */

#ifndef __COMMON_DATA_STRUCT_FIX_ARRAY_H_H___
#define __COMMON_DATA_STRUCT_FIX_ARRAY_H_H___

#include <assert.h>

#include "common/macro.h"

namespace ef {

template <class T>
class FixArray {
public:
    FixArray(int capacity)
        : _array(new T[capacity])
        , _size(0)
        , _capacity(capacity)
        , _free_list(new int[capacity])
        , _free_head(0)
    {
        assert(_array);
        assert(_free_list);
        for (int i=0; i<capacity-1; i++) {
            _free_list[i] = i+1;
        }
        _free_list[capacity-1] = -1;
    }

    virtual ~FixArray()
    {
        DELETE_ARRAY(_array);
        DELETE_ARRAY(_free_list);
    }

    /**
     * return idx
     */
    inline int Push(const T & t)
    {
        if (_free_head >= 0) {
            ++_size;
            int idx = _free_head;
            _free_head = _free_list[_free_head];
            _array[idx] = t;
            return idx;
        }
        return -1;
    }

    /**
     * idx: 一定是通过push得到的
     */
    inline int Pop(int idx) {
        --_size;
        // _array[idx].~T();       // 执行析构函数释放对象所占用资源
        // 调用对象的析构函数,可能会释放对象的部分资源,当重新利用该对象时,即对
        // 该对象重新赋值时,则会出现SEGMENT FAULT
        // 例如对象是,std::list,调用析构函数时,如果list中包含对象,析构函数会
        // 调用list的析构函数,释放list中存在的对象,不过list貌似没有把内部指针
        // 置null,当重新利用该对象时,赋值时会导致先释放原有list中的对象,此时
        // list内部包含野指针,进程就直接挂了
        _free_list[idx] = _free_head;
        _free_head = idx;
        return 0;
    }

    inline T & operator[](int idx) { return _array[idx]; }
    inline int Size() const { return _size; }
    inline int Capacity() const { return _capacity; }

private:
    T *_array;
    int _size;
    int _capacity;
    int *_free_list;
    int _free_head;
};

} /* namespace ef */


#endif /* __COMMON_DATA_STRUCT_FIX_ARRAY_H__ */
