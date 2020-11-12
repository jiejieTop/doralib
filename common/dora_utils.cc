/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-07 09:45:26
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-11 10:07:52
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include "stdint.h"
#include "stddef.h"
#include "time.h"

#include "dora_log.h"
#include "dora_error.h"
#include "dora_utils.h"


namespace doralib {
    
unsigned int utils_hash(char *key)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*key) {
        hash = hash * seed + (*key++);
    }

    return (hash & 0x7FFFFFFF);
}


int utils_set_deadline(struct timespec *deadline, long msecs)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
        return DORA_NOTHING_TO_READ;

    long secs = msecs / 1000;
    msecs = msecs % 1000;
    long add = 0;
    msecs = msecs * 1000 * 1000 + ts.tv_nsec;
    add = msecs / (1000 * 1000 * 1000);
    deadline->tv_sec = (ts.tv_sec + add + secs);
    deadline->tv_nsec = msecs % (1000 * 1000 * 1000);

    return DORA_SUCCESS;
}

bool utils_deadline_is_expire(const struct timespec *deadline)
{
	struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
        return false;

    if (deadline->tv_sec == ts.tv_sec)
        return deadline->tv_nsec < ts.tv_nsec;
    return deadline->tv_sec < ts.tv_sec;
}



}

