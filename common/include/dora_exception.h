/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-11-04 17:47:18
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#ifndef _DORA_EXCEPTION_H_
#define _DORA_EXCEPTION_H_

#include <string>
#include "dora_log.h"

#define DORA_EXCEPTION(msg) doralib::exception((char *)__FILE__, __LINE__, (char *)msg, true)

namespace doralib {
using std::string;
/**
 * @brief This class is instantiated and thrown when an error occurs.
 * If there's an error somewhere in doralib++, the function in which the error
 * occurs /always/ throws a `exception` object showing why the error
 * occurred.
 */

void exception(const string & file, int line, const string& message, bool show_errno);
void exception(char *file, int line, char *message, bool show_errno);


}  // namespace doralib
#endif
