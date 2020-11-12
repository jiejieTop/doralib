/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-03 17:33:18
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-11 13:24:17
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_mheap.h"
#include "dora_log.h"
#include "dora_error.h"

#include <chrono>

using doralib::mheap;

#define DORA_STRT_CPRT  { auto scprt = std::chrono::steady_clock::now();

#define DORA_END_CPRT   auto ecprt = std::chrono::steady_clock::now();  \
                        std::chrono::duration<double, std::micro> elapsed = ecprt - scprt;  \
                        DORA_LOG_DEBUG("code runtime : {:03.06f} us", elapsed.count()); }

int main()
{
    DORA_LOG_SET_DEBUG_LEVEL
    
    DORA_LOG_INFO("========================= mheap =========================");

    char *ptr[10];
    auto mheap = new doralib::mheap;

    mheap->mheap_print_info();
    
    DORA_LOG_INFO("========================= alloc =========================");

    for (int i = 0; i < 10; i++) {

        DORA_STRT_CPRT
        
        ptr[i] = (char *)mheap->mheap_alloc(128 + i);

        DORA_END_CPRT

        DORA_ROBUSTNESS_CHECK(ptr[i], -1);
        memset(ptr[i], 1, 128 + i);
    }
    mheap->mheap_print_info();

    DORA_LOG_INFO("========================= free =========================");

    for (int i = 0; i < 10; i++) {
        mheap->mheap_free(ptr[i]);
    }

    mheap->mheap_print_info();

    DORA_LOG_INFO("========================= success =========================");

    delete mheap;

    sleep(1);

    return 0;
}


