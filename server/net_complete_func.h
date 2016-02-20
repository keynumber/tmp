/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#ifndef __SERVER_NET_COMPLETE_FUNC_H_H___
#define __SERVER_NET_COMPLETE_FUNC_H_H___

namespace ef
{

struct PacketHeader {
    int length;         // length为包请求内容大小,不包括包头大小
    unsigned int request_id;
};

// buf 为数据内容指针
// len 为数据长度
// theory_len 完整包应该的长度
// 返回 > 0, 表示一个完整包的长度
// 返回 = 0, 表示包不完整,如果包长大于最小数据包的长度,则theory_len为理论包长
// 返回 < 0, 数据出错,不符合包规范
// note: 只有当能计算出包长时,才会填充改变theory_len
typedef int (*ppacket_len_func)(char *buf, int len, int * theoy_len);
// 数据包头的长度,根据这个长度的数据就能知道整个请求包大小
typedef int (*pheader_len_func)();

int packet_len_func(char *buf, int len, int * theoy_len);
int header_len_func();

} /* namespace ef */

#endif /* __SERVER_NET_COMPLETE_FUNC_H__ */
