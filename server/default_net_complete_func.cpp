/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#include "default_net_complete_func.h"

#include "global_configure.h"
#include "common/macro.h"

extern ef::GlobalConfigure gGlobalConfigure;

namespace ef
{

#define DEFAULT_MINMIMUM_PACKET_LENGTH sizeof(uint32_t)

int default_net_complete_func(char *buf, uint32_t len, uint32_t * theoy_len)
{
    // TODO ntoh
    if (unlikely(len < DEFAULT_MINMIMUM_PACKET_LENGTH))
        return 0;

    uint32_t packet_len = *(uint32_t*)buf;
    if (unlikely(len > gGlobalConfigure.max_packet_size)) {
        return -1;
    }

    *theoy_len = packet_len;
    if (len >= packet_len)
        return packet_len;
    return 0;
}

int default_minimum_packet_len_func()
{
    return DEFAULT_MINMIMUM_PACKET_LENGTH;
}

} /* namespace ef */
