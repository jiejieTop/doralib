/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-06 14:29:27
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-07 12:30:24
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
    
    auto timer = new doralib::timer;

    timer->add(5000, timer_action);
    timer->add(1100, timer_action);
    timer->add(1200, timer_action);
    timer->add(2000, timer_action);
    timer->add(3000, timer_action);
    timer->add(4000, timer_action);
    timer->add(5000, timer_action);
    timer->add(200, timer_action);
    timer->add(3020, timer_action);
    timer->add(2000, timer_action, DORA_TIMER_MODE_PERIOD);
    

    string line;
    getline(cin, line);

    return 0;
}


