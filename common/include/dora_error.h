/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-10-30 11:55:17
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#ifndef _DORA_ERROR_H_
#define _DORA_ERROR_H_

#include "dora_log.h"

namespace doralib 
{

typedef enum dora_errors {
    DORA_NULL_VALUE_ERROR                             = -11,      /* value is null */
    DORA_NOTHING_TO_READ_ERROR                        = -10,
    DORA_SOCKET_BIND_FAIL_ERROR                       = -9,
    DORA_MEM_NOT_ENOUGH_ERROR                         = -8,
    DORA_SEND_PACKET_ERROR                            = -7,
    DORA_CONNECT_FAILED_ERROR                         = -6,
    DORA_SOCKET_FAILED_ERROR                          = -5,
    DORA_SOCKET_UNKNOWN_HOST_ERROR                    = -4,
    DORA_PACKET_BUFFER_TOO_SHORT                      = -3,
    DORA_PACKET_READ_ERROR                            = -2,
    DORA_FAILED_ERROR                                 = -1,
    DORA_SUCCESS_ERROR                                = 0
} dora_errors_t;

#define DORA_ROBUSTNESS_CHECK(item, err) if (!(item)) {                                             \
        DORA_LOG_ERROR("%s:%d %s()... check for error.", __FILE__, __LINE__, __FUNCTION__);         \
        return err; }


}

#endif



