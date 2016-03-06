/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#include "net_complete_func.h"

#include <arpa/inet.h>

#include "common/macro.h"

namespace ef
{

const int max_packet_size = 100 * 1024;
const int min_packet_size = sizeof(PacketHeader);

int packet_len_func(char *buf, int len)
{
    // 大于最大包长
    if (unlikely(len > max_packet_size)) {
        return -1;
    }

    // 小于最小包长
    if (unlikely(len < min_packet_size)) {
        return 0;
    }

    // TODO ntoh
    PacketHeader * header = (PacketHeader*)buf;
    return header->length + sizeof(PacketHeader);
}

int header_len_func()
{
    return min_packet_size;
}

} /* namespace ef */
