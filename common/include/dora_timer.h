/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-06 12:05:38
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-06 19:35:15
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#ifndef _DORA_TIMER_H_
#define _DORA_TIMER_H_

#include <chrono>
#include <functional>
#include <thread>
#include <queue>
#include <mutex>

#include <semaphore.h>

namespace doralib {

#define DORA_TIMER_MODE_ONCE         (0x00)
#define DORA_TIMER_MODE_PERIOD       (0x01)

typedef void (*timer_action_t)(void *arg);

struct timer_worker {
    int                     id;             //定时事件的唯一标示id
    int                     mode;           //是否是重复执行事件
    long                    interval;       //事件的触发间隔，在重复事件中会用到这个属性
    struct timespec         deadline;
    timer_action_t          action;         //触发的事件
    void                    *arg;
    
    /* use priority queue minimum heap */
    bool operator < (timer_worker x) const { 
        if (this->deadline.tv_sec == x.deadline.tv_sec) 
            return this->deadline.tv_nsec > x.deadline.tv_nsec;
        return this->deadline.tv_sec > x.deadline.tv_sec;
    }
};

class timer {
private:
    uint32_t                            _id;
    pthread_t                           _thread;
    std::priority_queue<timer_worker>   _pq;
    sem_t                               _sem;
    std::mutex                          _mutex;
    // void loop();
public:
    timer();
    ~timer();

    bool _is_running;
    
    /**
     * add a timer 
     * @param interval timer interval
     * @param action timed actions
     * @param mode hether to execute repeatedly, the default is not to execute repeatedly
     * @return the id of the timer, which can be deleted according to this id
     */
    uint32_t add(long interval, timer_action_t action, int mode = DORA_TIMER_MODE_ONCE);
    void del(uint32_t id);
    int timer_wait(void);
};

}



#endif
