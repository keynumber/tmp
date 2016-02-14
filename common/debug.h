/*
 * Author: number
 * Date  : Feb 12, 2016
 */

#ifndef __SERVER_DEBUG_H_H___
#define __SERVER_DEBUG_H_H___

#define __SERVER_debug___

#ifdef __SERVER_debug___
#include "common/logger.h"
#define DEBUG(fmt, ...) LogFrame(fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#endif /* __SERVER_DEBUG_H__ */
