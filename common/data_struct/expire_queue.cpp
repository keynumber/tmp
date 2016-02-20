/*
 * Author: number
 * Date  : Feb 10, 2016
 */

#include "expire_queue.h"

#include <assert.h>
#include <string.h>

namespace ef {

ExpireQueue::ExpireQueue(int capacity)
    : _queue_pre(new int[capacity+1])
    , _queue_next(new int[capacity+1])
    , _capacity(capacity)
    , _tail(capacity)
{
    assert(_queue_pre);
    assert(_queue_next);
    memset(_queue_pre, ~0, sizeof(int)*(_capacity+1));
    memset(_queue_next, ~0, sizeof(int)*(_capacity+1));
}

ExpireQueue::~ExpireQueue()
{
    DELETE_ARRAY(_queue_pre);
    DELETE_ARRAY(_queue_next);
}

void ExpireQueue::Activate(int idx)
{
    if (unlikely(idx < 0 || idx >= _capacity)) {
        return;
    }

    if (idx == _tail) {
        return;
    }

    int pre = _queue_pre[idx];
    int next = _queue_next[idx];

    // internal node, take
    if (next > 0) {
        _queue_next[pre] = next;
        _queue_pre[next] = pre;
    }

    // append
    _queue_next[_tail] = idx;
    _queue_pre[idx] = _tail;
    _queue_next[idx] = -1;
    _tail = idx;
}

void ExpireQueue::Erase(int idx)
{
    if (unlikely(idx < 0 || idx >= _capacity)) {
        return;
    }

    // node not in expire queue
    if (_queue_pre[idx] < 0) {
        return;
    }

    int pre = _queue_pre[idx];
    int next = _queue_next[idx];

    if (idx == _tail) {
        _tail = pre;
    }

    _queue_next[pre] = next;
    // internal node, set next node pre pointer
    if (next > 0) {
        _queue_pre[next] = pre;
    }

    _queue_pre[idx] = -1;
    _queue_next[idx] = -1;
}

} /* namespace ef */
