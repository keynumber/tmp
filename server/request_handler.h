/*
 * Author: number
 * Date  : Feb 16, 2016
 */

#ifndef __SERVER_REQUEST_HANDLER_H_H___
#define __SERVER_REQUEST_HANDLER_H_H___

#include "comm_struct.h"
#include "../common/rc_buf.h"

// TODO 测试后需要删除

namespace ef {

void HandleClientRequest(int handler_id, const ClientReqPack &req);

} /* namespace ef */

#endif /* __SERVER_REQUEST_HANDLER_H__ */
