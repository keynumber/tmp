/*
 * Author: number
 * Date  : Nov 19, 2015
 */

#ifndef __COMMON_UTIL_H_H___
#define __COMMON_UTIL_H_H___

#include <string>

namespace ef {

std::string JoinPath(const std::string &p1, const std::string &p2);

inline std::string IpToString(uint32_t ip)
{
    char buf[32];
    snprintf(buf, 32, "%d.%d.%d.%d", (ip>>0 )&0xFF,
                                     (ip>>8 )&0xFF,
                                     (ip>>16)&0xFF,
                                     (ip>>24)&0xFF);
    return std::string(buf);
}

} /* namespace ef */

#endif /* __COMMON_UTIL_H__ */
