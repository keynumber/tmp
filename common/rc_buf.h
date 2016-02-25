/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#ifndef __COMMON_RC_BUF_H_H___
#define __COMMON_RC_BUF_H_H___

#include <stdint.h>
#include <assert.h>

#include "macro.h"

// #define __RC_DEBUG__
#ifdef __RC_DEBUG__
#include <stdio.h>
#define RCDEBUG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define RCDEBUG(fmt, ...)
#endif

namespace ef {

// 只用来维护buf的生存期，其他不管
class RcBuf {
public:
    RcBuf()
        : buf(nullptr)
        , offset(0)
        , len(0)
        , _counter(nullptr)
        , _is_released(true) {}

    RcBuf(int buflen)
        : buf(new char[buflen])
        , offset(0)
        , len(buflen)
        , _counter(new uint32_t(0))
        , _is_released(false)
    {
        assert(buf);
        assert(_counter);
        __sync_fetch_and_add(_counter, 1);
        RCDEBUG("Construct with len %d, offset %d, counter: %d\n", len, offset,  *_counter);
    }

    RcBuf(const RcBuf &rcbuf) {
        buf = rcbuf.buf;
        offset = rcbuf.offset;
        len = rcbuf.len;
        _counter = rcbuf._counter;
        if (_counter) {
            _is_released = false;
            __sync_fetch_and_add(_counter, 1);
            RCDEBUG("copy Construct with len %d, offset %d, counter: %d\n", len, offset,  *_counter);
        } else {
            _is_released = true;
        }
    }

    RcBuf & operator=(const RcBuf &rcbuf) {
        Release();

        buf = rcbuf.buf;
        offset = rcbuf.offset;
        len = rcbuf.len;
        _counter = rcbuf._counter;
        if (_counter) {
            _is_released = false;
            __sync_fetch_and_add(_counter, 1);
            RCDEBUG("operator = with len %d, offset %d, counter: %d\n", len, offset,  *_counter);
        } else {
            _is_released = true;
        }
        return *this;
    }

    virtual ~RcBuf() {
        Release();
    }

    void Release() {
        if (!_is_released) {
            // 防止release后，析构函数再次release
            _is_released = true;

            int count = __sync_sub_and_fetch(_counter, 1);
            RCDEBUG("release with len %d, offset %d, counter: %d\n", len, offset,  *_counter);
            // 此处不能使用使用*_count去做判断，因为重新获取*_count的值，其他的线程可能已经将这个值
            // 重新改写，从而可能造成double free,从而导致coredump
            // if (*_count == 0) {
            if (count == 0) {
                RCDEBUG("delete memory len %d, offset %d, counter: %d\n", len, offset,  *_counter);
                DELETE_ARRAY(buf);
                DELETE_POINTER(_counter);
            } else {
                _counter = nullptr;
                buf = nullptr;
                offset = 0;
                len = 0;
            }
        }
    }

    void Copy(const RcBuf & rcbuf, int o, int l)
    {
        Release();

        _counter = rcbuf._counter;
        buf = rcbuf.buf;
        offset = o;
        len = l;
        if (_counter) {
            _is_released = false;
            __sync_fetch_and_add(_counter, 1);
            RCDEBUG("copy with len %d, offset %d, counter: %d\n", len, offset,  *_counter);
        } else {
            _is_released = true;
        }
    }

public:
    char * buf;             // 需要传递的buf
    int offset;        // 内容在buffer中的偏移
    int len;           // 内容的长度

private:
    uint32_t * _counter;
    bool _is_released;
};

} /* namespace ef */

#endif /* __COMMON_RC_BUF_H__ */
