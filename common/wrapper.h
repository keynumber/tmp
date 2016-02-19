/*
 * Author: number
 * Date  : Nov 8, 2015
 */

#ifndef __COMMON_WRAPPER_H_H___
#define __COMMON_WRAPPER_H_H___

#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <string>

#include "macro.h"

namespace ef {

inline ssize_t safe_read(int fd, void *buf, size_t count) {
    ssize_t ret;
    do {
        ret = read(fd, buf, count);
    } while (unlikely(ret < 0 && errno == EINTR));
    return ret;
}

inline ssize_t safe_readv(int fd, struct iovec *iov, size_t iovcnt) {
    ssize_t ret;
    do {
        ret = readv(fd, iov, iovcnt);
    } while (unlikely(ret < 0 && errno == EINTR));
    return ret;
}

inline ssize_t safe_write(int fd, const void *buf, size_t count)
{
    ssize_t ret;
    do {
        ret = write(fd, buf, count);
    } while (unlikely(ret < 0 && errno == EINTR));
    return ret;
}

inline ssize_t safe_writev(int fd, struct iovec *iov, size_t iovcnt) {
    ssize_t ret;
    do {
        ret = writev(fd, iov, iovcnt);
    } while (unlikely(ret < 0 && errno == EINTR));
    return ret;
}

inline int set_nonblock(int fd)
{
    int ret = 0;
    do {
        ret = fcntl(fd, F_GETFL);
    } while (unlikely(ret <0 && errno == EINTR));

    if (ret < 0) {
        return -1;
    }

    do {
        ret = fcntl(fd, F_SETFL, ret | O_NONBLOCK | O_NDELAY);
    } while (unlikely(ret <0 && errno == EINTR));

    return ret;
}

inline int safe_close(int fd)
{
    int ret;
    do {
        ret = close(fd);
    } while (unlikely(ret < 0 && errno == EINTR));
    return ret;
}

std::string safe_strerror(int errorno);

} /* namespace ef */

#endif /* __COMMON_WRAPPER_H_H___*/
