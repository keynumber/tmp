/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#ifndef __SERVER_DEFAULT_NET_COMPLETE_FUNC_H_H___
#define __SERVER_DEFAULT_NET_COMPLETE_FUNC_H_H___

#include <stdint.h>

namespace ef
{

// buf 为数据内容指针
// len 为数据长度
// theory_len 完整包应该的长度
// 返回 > 0, 表示一个完整包的长度
// 返回 = 0, 表示包不完整,如果包长大于最小数据包的长度,则theory_len为理论包长
// 返回 < 0, 数据出错,不符合包规范
int default_net_complete_func(char *buf, uint32_t len, uint32_t * theoy_len);
// 最小数据包的长度,根据这个长度的数据就能知道包大小
int default_minimum_packet_len_func();

} /* namespace ef */

#endif /* __SERVER_DEFAULT_NET_COMPLETE_FUNC_H__ */