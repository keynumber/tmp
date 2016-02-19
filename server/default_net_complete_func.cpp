/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#include "default_net_complete_func.h"

#include "common/macro.h"

namespace ef
{

const int max_packet_size = 20 * 1024;
const int min_packet_size = sizeof(uint32_t);

int default_net_complete_func(char *buf, uint32_t len, uint32_t * theoy_len)
{
    // TODO ntoh
    if (unlikely(len < min_packet_size))
        return 0;

    uint32_t packet_len = *(uint32_t*)buf;
    if (unlikely(len > max_packet_size)) {
        return -1;
    }

    *theoy_len = packet_len;
    if (len >= packet_len)
        return packet_len;
    return 0;
}

int default_minimum_packet_len_func()
{
    return min_packet_size;
}

} /* namespace ef */
