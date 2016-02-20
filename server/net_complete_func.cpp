/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#include "net_complete_func.h"

#include "common/macro.h"

namespace ef
{

const int max_packet_size = 100 * 1024;
const int min_packet_size = sizeof(PacketHeader);

int packet_len_func(char *buf, int len, int * theoy_len)
{
    // TODO ntoh
    if (unlikely(len < min_packet_size))
        return 0;

    PacketHeader * header = (PacketHeader*)buf;
    if (unlikely(len > max_packet_size)) {
        return -1;
    }

    *theoy_len = header->length;
    if (len >= header->length)
        return header->length;
    return 0;
}

int header_len_func()
{
    return min_packet_size;
}

} /* namespace ef */
