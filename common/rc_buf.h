/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#ifndef __COMMON_RC_BUF_H_H___
#define __COMMON_RC_BUF_H_H___

#include <stdint.h>
#include <assert.h>

#include "macro.h"

namespace ef {

#define RCDEBUG(fmt, ...) printf(fmt, ##__VA_ARGS__)

struct TransferObj {
    char * buf = nullptr;           // 需要传递的buf
    uint32_t * counter = nullptr;   // 引用计数
    uint32_t offset = 0;            // 内容在buffer中的偏移
    uint32_t len = 0;               // 内容的长度
};

// 只用来维护buf的生存期，其他不管
class RcBuf {
public:
    RcBuf()
        : buf(nullptr)
        , offset(0)
        , len(0)
        , _counter(nullptr)
        , _is_released(true) {}

    RcBuf(uint32_t buflen)
        : buf(new char[buflen])
        , offset(0)
        , len(buflen)
        , _counter(new uint32_t(0))
        , _is_released(false)
    {
        assert(buf);
        assert(_counter);
        __sync_fetch_and_add(_counter, 1);

        RCDEBUG("Construct, counter: %d\n", *_counter);
    }

    virtual ~RcBuf() {
        Release();
    }

    void Release() {
        if (!_is_released) {
            // 防止release后，析构函数再次release
            _is_released = true;

            int count = __sync_sub_and_fetch(_counter, 1);
            RCDEBUG("Release, counter: %d\n", *_counter);
            // 此处不能使用使用*_count去做判断，因为重新获取*_count的值，其他的线程可能已经将这个值
            // 重新改写，从而可能造成double free,从而导致coredump
            // if (*_count == 0) {
            if (count == 0) {
                RCDEBUG("delete memory\n");
                DELETE_ARRAY(buf);
                DELETE_POINTER(_counter);
            }
        }
    }

    void Copy(const RcBuf & rcbuf, uint32_t o, uint32_t l)
    {
        if (!_is_released) {
            Release();
        }

        _counter = rcbuf._counter;
        _is_released = false;
        buf = rcbuf.buf;
        offset = o;
        len = o;

        __sync_fetch_and_add(_counter, 1);
    }

    // 为了让构造函数行为保持一致，因此不为该类初始化提供构造函数
    // 因为构造函数都会对_counter自加
    inline uint32_t ToTransferObj(TransferObj *obj, uint32_t o, uint32_t l)
    {
        obj->buf = buf;
        obj->offset = o;
        obj->len = l;
        obj->counter = _counter;

        // 放入队列前，需要增加引用计数，否则可能出现，消费者还没有消费任何消息
        // 还没有被处理，io线程释放了当前buffer导致buffer被释放，从而出现错误
        // 或者，部分被消费完，引用计数又被减为0，队列中还存在待消费数据，错误
        // 因此，Parse不需要增加引用计数
        // ++*_counter;
        __sync_fetch_and_add(_counter, 1);

        RCDEBUG("ToTransferObj, counter: %d\n", *_counter);

        return sizeof(RcBuf);
    }

    inline void FromTransferObj(const TransferObj & obj) {
        if (!_is_released) {
            Release();
        }

        buf = obj.buf;
        len = obj.len;
        offset = obj.offset;
        _counter = obj.counter;
        _is_released = false;

        RCDEBUG("Parse, counter: %d\n", *_counter);
    }

public:
    char * buf;             // 需要传递的buf
    uint32_t offset;        // 内容在buffer中的偏移
    uint32_t len;           // 内容的长度

private:
    uint32_t * _counter;
    bool _is_released;
};

} /* namespace ef */

#endif /* __COMMON_RC_BUF_H__ */
