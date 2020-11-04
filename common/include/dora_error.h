/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-11-04 19:08:26
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#ifndef _DORA_ERROR_H_
#define _DORA_ERROR_H_

#include "dora_log.h"

namespace doralib {

typedef enum dora_errors {
    /* other error */
    DORA_NOTHING_TO_READ                            = -500,     
    /* buffer error */
    DORA_BUFFER_TOO_LONG                            = -401,      
    DORA_BUFFER_TOO_SHORT                           = -400,
    /* memory error */
    DORA_MEM_NOT_ENOUGH                             = -300,      
    /* socket error */
    DORA_SOCKET_SEND_FAILED                         = -205,      
    DORA_SOCKET_READ_FAILED                         = -204,
    DORA_SOCKET_BIND_FAILED                         = -203,
    DORA_SOCKET_CONNECT_FAILED                      = -202,
    DORA_SOCKET_UNKNOWN_HOST                        = -201,
    DORA_SOCKET_FAILED                              = -200,
    /* value error */
    DORA_VALUE_ALREADY_NOT_EXIST                    = -103,       
    DORA_VALUE_ALREADY_EXIST                        = -102,
    DORA_VALUE_OVERFLOW                             = -101,
    DORA_VALUE_INVALID                              = -100,       /* value is invalid */
    DORA_FAILED                                     = -1,
    DORA_SUCCESS                                    = 0
} dora_errors_t;

#define DORA_VOID   

#define DORA_ROBUSTNESS_CHECK(item, err) if (!(item)) {                                             \
        DORA_LOG_ERROR("{}:{} {}()... check for error.", __FILE__, __LINE__, __FUNCTION__);         \
        return err; }


}

#endif



