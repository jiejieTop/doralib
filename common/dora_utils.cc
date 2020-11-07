/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-07 09:45:26
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-07 09:50:15
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include "stdint.h"
#include "stddef.h"

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

}

