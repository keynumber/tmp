/*
 * Author: number
 * Date  : Feb 16, 2016
 */

#ifndef __SERVER_REQUEST_HANDLER_H_H___
#define __SERVER_REQUEST_HANDLER_H_H___

#include "comm_struct.h"
#include "../common/rc_buf.h"

namespace ef {

class RequestHandler {
public:
    RequestHandler();
    virtual ~RequestHandler();

    virtual int RequestHandler(const FdInfo & fdinfo, const RcBuf & rcbuf) = 0;
    virtual int RspToClient(const FdInfo & fdinfo, const RcBuf & rcbuf) = 0;
};

} /* namespace ef */

#endif /* __SERVER_REQUEST_HANDLER_H__ */
