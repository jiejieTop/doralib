/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-06 14:29:27
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-08 11:11:06
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_log.h"
#include "dora_timer.h"

#include <iostream>

using namespace std;
using namespace doralib;

void timer_action(void *arg)
{
    struct timer_worker *tworker = (struct timer_worker *)arg;
    DORA_LOG_INFO("timer action : id is {}", tworker->id);
    
}


int main()
{
    // DORA_LOG_SET_DEBUG_LEVEL
    DORA_LOG_INFO("========================= timer =========================");
    
    auto timer = new doralib::timer;

    timer->add(500, timer_action);
    timer->add(110, timer_action);
    timer->add(120, timer_action);
    timer->add(200, timer_action);
    timer->add(300, timer_action);
    timer->add(400, timer_action);
    timer->add(500, timer_action);
    timer->add(20, timer_action);
    timer->add(310, timer_action);
    timer->add(200, timer_action, DORA_TIMER_MODE_PERIOD);
    
    sleep(1);

    DORA_LOG_INFO("========================= success =========================");

    return 0;
}


