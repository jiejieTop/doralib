/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-06 12:10:37
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-07 12:35:47
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include "stdint.h"
#include "stddef.h"

#include "dora_timer.h"
#include "dora_log.h"
#include "dora_error.h"
#include "dora_exception.h"


namespace doralib {

static int timer_set_deadline(struct timespec *deadline, long msecs)
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

static bool timer_is_expire(const struct timespec *deadline)
{
	struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
        return false;

    if (deadline->tv_sec == ts.tv_sec)
        return deadline->tv_nsec < ts.tv_nsec;
    return deadline->tv_sec < ts.tv_sec;
}

static int timer_handler(const struct timer_worker *tworker)
{
    if (timer_is_expire(&tworker->deadline)) {
        if (NULL != tworker->action)
            tworker->action((void*)tworker);
        return DORA_SUCCESS;
    }
    return DORA_FAILED;
}


static void *timer_thread(void *arg)
{
    timer *t = (timer *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); // allow thread to exit 
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // settings cancel ASYNCHRONOUS
    DORA_LOG_DEBUG("timer thread running ...");

    while (t->_is_running) {
        t->timer_wait();
    }
    
    return NULL;
}


timer::timer()
    : _id(0)
    , _is_running(true)
{
    sem_init(&_sem, 0, 0);

    int ret = pthread_create(&_thread, NULL, &timer_thread, this);
    if(ret != 0) {
        DORA_EXCEPTION("create timer thread error");
    }
}

timer::~timer()
{
    _is_running = false;
    
    pthread_cancel(_thread);
    pthread_detach(_thread);
    
    sem_destroy(&_sem);
}

int timer::timer_wait()
{
    int ret = DORA_FAILED;
    /* wait for the semaphore to see if the waiting time needs to be updated */
    if (!_pq.empty()) {
        struct timer_worker tworker = _pq.top();
        ret = sem_timedwait(&_sem, &tworker.deadline);
    } else {
        ret = sem_wait(&_sem);
    }

    if (ret != 0) {
        /* the timer has expired and the timer callback function needs to be processed */
        struct timer_worker tworker = _pq.top();
        DORA_LOG_DEBUG("timer timeout, interval {}", tworker.interval);
        
        int ret = timer_handler(&tworker);
        if (DORA_SUCCESS == ret) {
            std::unique_lock<std::mutex> lock(_mutex);
            _pq.pop();

            if (DORA_TIMER_MODE_PERIOD == tworker.mode) {
                timer_set_deadline(&tworker.deadline, tworker.interval);
                _pq.push(tworker);
            }
        }
    }
    return ret;
}

uint32_t timer::add(long interval, timer_action_t action, int mode)
{
    bool need_notice = false;
    struct timer_worker tworker;
    tworker.mode = mode;
    tworker.action = action;
    tworker.interval = interval;
    timer_set_deadline(&tworker.deadline, interval);

    if (_pq.size()) {
        struct timer_worker tworker_top = _pq.top();
        if (tworker_top < tworker) {
            need_notice = true;
        } 
    } else {
        need_notice = true;
    }

    {
        std::unique_lock<std::mutex> lock(_mutex);
        _id ++;
        tworker.id = _id;
        _pq.push(tworker);
    }

    if (need_notice)
        sem_post(&_sem);

    return tworker.id;
}

void timer::del(uint32_t id)
{

    
}


}


