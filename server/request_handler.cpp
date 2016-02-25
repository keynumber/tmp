/*
 * Author: number
 * Date  : Feb 16, 2016
 */

#include <stdio.h>

#include "request_handler.h"
#include "net_complete_func.h"
#include "message_center.h"

namespace ef {

void HandleClientRequest(int handler_id, const ClientReqPack &req)
{
    const RcBuf & rcbuf = req.request_buf;

    PacketHeader * header = (PacketHeader*)(rcbuf.buf + rcbuf.offset);
    char * buf = rcbuf.buf + rcbuf.offset + sizeof(PacketHeader);

    printf("worker %d: get request from iohandler buf offset %d, buf len %d, content len: %d, request id: %d\n",
          handler_id, rcbuf.offset, rcbuf.len, header->length, header->request_id);
    printf("request buf: |%s|\n", buf);

    for (int i = 0; i<header->length; ++i) {
        if(buf[i] >= 'a' && buf[i] <= 'z')
            buf[i] = buf[i] - 'a' + 'A';
    }

    ServerRspPack rsp;
    rsp.fdinfo = req.fdinfo;
    rsp.handler_id = req.handler_id;
    rsp.response_buf = req.request_buf;
    MessageCenter::PostSvrRspToClient(req.handler_id, rsp);
}

} /* namespace ef */
