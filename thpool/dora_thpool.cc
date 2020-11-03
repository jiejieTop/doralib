/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-26 09:54:36
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-03 11:47:42
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_thpool.h"
#include "dora_log.h"

namespace doralib {

void thpool::init()
{
    DORA_LOG_DEBUG("thread pool init {} threads", (int)_threads.size());
    for (int i = 0; i < (int)_threads.size(); ++i) {
        _threads[i] = std::thread(worker(this, i));
    }
}

void thpool::deinit()
{
    DORA_LOG_DEBUG("thread pool deinit {} threads", (int)_threads.size());
    for (int i = 0; i < (int)_threads.size(); ++i) {
        _threads[i].~thread();
    }
}

thpool::thpool()
    : _shutdown(false)
    , _threads(std::vector<std::thread>(std::thread::hardware_concurrency()))
    
{
    init();
}

thpool::thpool(const int num)
    : _shutdown(false)
    , _threads(std::vector<std::thread>(num))
{
    init();
}

thpool::~thpool(void) 
{
    deinit();
}

// waits until threads finish their current task and shutdowns the pool
void thpool::shutdown() 
{
    _shutdown = true;
    _lock.notify_all();

    for (int i = 0; i < (int)_threads.size(); ++i) {
        if(_threads[i].joinable()) {
            _threads[i].join();
        }
    }
}

}



