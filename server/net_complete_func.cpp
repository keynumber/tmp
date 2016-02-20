/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#include "net_complete_func.h"

#include "common/macro.h"

namespace ef
{

const int max_packet_size = 20 * 1024;
const int min_packet_size = sizeof(int);

int packet_len_func(char *buf, int len, int * theoy_len)
{
    // TODO ntoh
    if (unlikely(len < min_packet_size))
        return 0;

    int packet_len = *(int*)buf;
    if (unlikely(len > max_packet_size)) {
        return -1;
    }

    *theoy_len = packet_len;
    if (len >= packet_len)
        return packet_len;
    return 0;
}

int header_len_func()
{
    return min_packet_size;
}

} /* namespace ef */
