/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-11-05 16:57:55
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#ifndef _DORA_LOG_H_
#define _DORA_LOG_H_

#include "spdlog/spdlog.h"

namespace doralib  {
#define DORA_LOG_TRACE_LEVEL        spdlog::level::level_enum::trace
#define DORA_LOG_DEBUG_LEVEL        spdlog::level::level_enum::debug
#define DORA_LOG_INFO_LEVEL         spdlog::level::level_enum::info
#define DORA_LOG_WARN_LEVEL         spdlog::level::level_enum::warn
#define DORA_LOG_ERROR_LEVEL        spdlog::level::level_enum::err
#define DORA_LOG_CRITICAL_LEVEL     spdlog::level::level_enum::critical
#define DORA_LOG_OFF_LEVEL          spdlog::level::level_enum::off

#define DORA_LOG_TRACE(msg, ...)    spdlog::trace(msg, ##__VA_ARGS__)
#define DORA_LOG_DEBUG(msg, ...)    spdlog::debug(msg, ##__VA_ARGS__)
#define DORA_LOG_INFO(msg, ...)     spdlog::info(msg, ##__VA_ARGS__)
#define DORA_LOG_WARN(msg, ...)     spdlog::warn(msg, ##__VA_ARGS__)
#define DORA_LOG_ERROR(msg, ...)    spdlog::error(msg, ##__VA_ARGS__)
#define DORA_LOG_CRITICAL(msg, ...) spdlog::critical(msg, ##__VA_ARGS__)
#define DORA_LOG_OFF(msg, ...)      spdlog::off(msg, ##__VA_ARGS__)

#define DORA_LOG_SET_LEVEL(l)       spdlog::set_level(l)

#define DORA_LOG_SET_TRACE_LEVEL    { DORA_LOG_SET_LEVEL(DORA_LOG_TRACE_LEVEL); }
#define DORA_LOG_SET_DEBUG_LEVEL    { DORA_LOG_SET_LEVEL(DORA_LOG_DEBUG_LEVEL); }
#define DORA_LOG_SET_INFO_LEVEL     { DORA_LOG_SET_LEVEL(DORA_LOG_INFO_LEVEL); }
#define DORA_LOG_SET_WARN_LEVEL     { DORA_LOG_SET_LEVEL(DORA_LOG_WARN_LEVEL); }
#define DORA_LOG_SET_ERROR_LEVEL    { DORA_LOG_SET_LEVEL(DORA_LOG_ERROR_LEVEL); }
#define DORA_LOG_SET_CRITICAL_LEVEL { DORA_LOG_SET_LEVEL(DORA_LOG_CRITICAL_LEVEL); }
#define DORA_LOG_SET_OFF_LEVEL      { DORA_LOG_SET_LEVEL(DORA_LOG_OFF_LEVEL); }

}

#endif // !_DORA_LOG_H_


