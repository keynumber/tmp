/*
 * Author: number
 * Date  : Feb 11, 2016
 */

#ifndef __COMMON_DATA_STRUCT_FIX_ARRAY_H_H___
#define __COMMON_DATA_STRUCT_FIX_ARRAY_H_H___

#include <assert.h>

namespace ef {

template <class T>
class FixArray {
public:
    FixArray(int capacity)
        : _array(new T[capacity])
        , _size(0)
        , _capacity(capacity)
        , _free_list(new int[capacity])
        , _free_head(0) {
        assert(_array);
        assert(_free_list);
        for (int i=0; i<capacity-1; i++) {
            _free_list[i] = i+1;
        }
        _free_list[capacity-1] = -1;
    }

    virtual ~FixArray() {
        delete [] _array;
        _array = nullptr;
        delete [] _free_list;
        _free_list = nullptr;
    }

    /**
     * return idx
     */
    int Push(const T & t) {
        if (_free_head >= 0) {
            ++_size;
            int idx = _free_head;
            _free_head = _free_list[_free_head];
            _array[idx] = t;
            return idx;
        }
        return -1;
    }

    inline int Pop(int idx) {
        --_size;
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
