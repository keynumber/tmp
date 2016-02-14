#include "wrapper.h"

#include <string.h>

namespace ef {

std::string safe_strerror(int errorno)
{
    static const int buflen =  1024;
    char buf[buflen];

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
    strerror_r(errorno, buf, buflen);
    const char *p = buf;
#else
    char *p = strerror_r(errorno, buf, buflen);
#endif

    return std::string(p);
}

} /* namespace ef */

