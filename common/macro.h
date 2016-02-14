/*
 * Author: number
 * Date  : Nov 1, 2015
 */

#ifndef __COMMON_MACRO_H_H___
#define __COMMON_MACRO_H_H___

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define DELETE_POINTER(p) do {          \
    delete p;                           \
    p = nullptr;                        \
} while (0)

#define DELETE_ARRAY(p) do {            \
    delete [] p;                        \
    p = nullptr;                        \
} while (0)

#endif /* __COMMON_MACRO_H__ */
