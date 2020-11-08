/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-03 17:33:18
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-08 11:08:36
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_mpool.h"
#include "dora_log.h"
#include "dora_error.h"

#include <chrono>

using doralib::mpool;

#define DORA_STRT_CPRT  { auto scprt = std::chrono::steady_clock::now();

#define DORA_END_CPRT   auto ecprt = std::chrono::steady_clock::now();  \
                        std::chrono::duration<double, std::micro> elapsed = ecprt - scprt;  \
                        DORA_LOG_INFO("code runtime : {:03.06f} us", elapsed.count()); }

int main()
{
    char *ptr[100];
    auto mpool = new doralib::mpool(1024, 100);
    
    DORA_LOG_INFO("========================= alloc =========================");

    for (int i = 0; i < 100; i++) {

        DORA_STRT_CPRT
        
        ptr[i] = (char *)mpool->mpool_alloc();

        DORA_END_CPRT

        DORA_ROBUSTNESS_CHECK(ptr[i], -1);
        memset(ptr[i], 1, 1024);
    }

    DORA_LOG_INFO("========================= free =========================");

    for (int i = 100; i > 0; i--) {
        mpool->mpool_free(ptr[i - 1]);
    }

    delete mpool;

    sleep(1);

    DORA_LOG_INFO("========================= success =========================");

    return 0;
}


