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
        , _free_tail(capacity-1)
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
     * 可以通过pre将对象串成list
     */
    inline int Push(int pre = -1)
    {
        if (_free_head >= 0) {
            int idx = _free_head;
            _free_head = _free_list[_free_head];
            _free_list[idx] = -1;
            // 没有判断pre是否大于0,直接写,造成写了指针头部之前的一些信息,
            // 导致存在于分配指针头部的系统管理信息被覆盖,然后析构时,
            // 碰到了奇怪的double free问题......
            if (pre >= 0) {
                _free_list[pre] = idx;
            }
            ++_size;
            return idx;
        }
        return -1;
    }

    /**
     * 只能释放整个列表
     * idx: 一定是通过push得到的头部节点的idx
     * 只能释放整个列表,而不能只释放部分节点
     */
    inline int PopList(int idx) {
        if (idx < 0) {
            return 0;
        }

        // _array[idx].~T();       // 执行析构函数释放对象所占用资源
        // 调用对象的析构函数,可能会释放对象的部分资源,当重新利用该对象时,即对
        // 该对象重新赋值时,则会出现SEGMENT FAULT
        // 例如对象是,std::list,调用析构函数时,如果list中包含对象,析构函数会
        // 调用list的析构函数,释放list中存在的对象,不过list貌似没有把内部指针
        // 置null,当重新利用该对象时,赋值时会导致先释放原有list中的对象,此时
        // list内部包含野指针,进程就直接挂了
        int tail = idx;
        int n = 1;
        while (_free_list[tail] > 0) {
            tail = _free_list[tail];
            ++n;
        }

        // 释放的对象放在空闲链的末尾
        // 因为,FixArray用来放FdInfo,可能出现请求发送到worker后,继续处理client的发送数据时出错,
        // 导致关闭fd,然后另外的连接使用了这个对象,导致数据错发,因此优先使用长时间未使用的对象
        // 因此,由于使用场景的问题,释放的对象放在空闲链的末尾,从而优先利用长时间为用过的对象
        if (_free_head < 0) {
            _free_head = idx;
            _free_tail = tail;
        } else {
            _free_list[_free_tail] = idx;
            _free_tail = tail;
        }
        _size -= n;
        return 0;
    }

    inline int Pop(int idx) {
        if (idx < 0) {
            return 0;
        }

        if (_free_head < 0) {
            _free_head = idx;
            _free_tail = idx;
        } else {
            _free_list[_free_tail] = idx;
            _free_tail = idx;
        }
        --_size;
        return 0;
    }

    // idx必须是push返回的
    // 如果是结尾返回-1,否则返回下标
    inline int GetNext(int idx) { return _free_list[idx]; }

    inline T & operator[](int idx) { return _array[idx]; }
    inline int Size() const { return _size; }
    inline int Capacity() const { return _capacity; }

private:
    T *_array;
    int _size;
    int _capacity;
    int *_free_list;
    int _free_head;
    int _free_tail;
};

} /* namespace ef */


#endif /* __COMMON_DATA_STRUCT_FIX_ARRAY_H__ */
