/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-07 09:48:32
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-10 12:09:31
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#ifndef _DORA_UTILS_H_
#define _DORA_UTILS_H_

namespace doralib {

unsigned int utils_hash(char *key);
int utils_set_deadline(struct timespec *deadline, long msecs);
bool utils_deadline_is_expire(const struct timespec *deadline);

}

#endif


