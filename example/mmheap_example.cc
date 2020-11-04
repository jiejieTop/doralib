/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-03 17:33:18
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-04 15:53:19
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
                        DORA_LOG_INFO("code runtime : {:03.06f} us", elapsed.count()); }

int main()
{
    int ret = -1;
    char *ptr[500];
    auto mheap = new doralib::mheap;

    auto _mheap_ptr = new char[MHEAP_DEFAULT_SIZE];
    DORA_ROBUSTNESS_CHECK(_mheap_ptr, -1);
    mheap->mheap_pool_add(_mheap_ptr, MHEAP_DEFAULT_SIZE);

    
    DORA_LOG_INFO("========================= alloc =========================");

    for (int i = 0; i < 500; i++) {

        DORA_STRT_CPRT
        
        ptr[i] = (char *)mheap->mheap_alloc(128 + i);

        DORA_END_CPRT

        DORA_ROBUSTNESS_CHECK(ptr[i], -1);
        memset(ptr[i], 1, 128 + i);

        DORA_LOG_INFO("ptr is {:p}, size : {}", ptr[i], (128 + i));
        mheap->mheap_print_info();
    }

    DORA_LOG_INFO("========================= free =========================");

    for (int i = 500; i > 0; i--) {
        mheap->mheap_free(ptr[i - 1]);
    }

    DORA_LOG_INFO("========================= success =========================");

    mheap->mheap_print_info();
    ret = mheap->mheap_pool_del(_mheap_ptr);
    DORA_LOG_INFO("delete pool ret is {}", ret);
    mheap->mheap_print_info();

    delete _mheap_ptr;
    delete mheap;

    sleep(1);

    return 0;
}


