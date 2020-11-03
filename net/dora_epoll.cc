/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-29 19:05:37
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-02 17:32:56
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_epoll.h"
#include "dora_error.h"
#include <functional>

namespace doralib {

using doralib::dgram_socket;

const int maxevents = 16;

static void epoller_dgram_events_handler(epoller_events_t *events, int happen_events)
{
    DORA_LOG_DEBUG("happen {} events", happen_events);

    if ((happen_events & EPOLLHUP) && !(happen_events & EPOLLIN)) {
        if (NULL != events->u.dgram_event.close_cb) {
            events->u.dgram_event.close_cb(events->u.dgram_event.dgram);
        }
    }
    if (happen_events & EPOLLERR) {
        if (NULL != events->u.dgram_event.error_cb) {
            events->u.dgram_event.error_cb(events->u.dgram_event.dgram);
        }
    }
    if (happen_events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (NULL != events->u.dgram_event.read_cb) {
            events->u.dgram_event.read_cb(events->u.dgram_event.dgram);
        }
    }
    if (happen_events & EPOLLOUT) {
        if (NULL != events->u.dgram_event.write_cb) {
            events->u.dgram_event.write_cb(events->u.dgram_event.dgram);
        }
    }
}


static void epoller_events_handler(epoller_events_t *events, int happen_events)
{
    switch (events->type)
    {
    case DORA_SOCK_UDP:
        epoller_dgram_events_handler(events, happen_events);
        break;
    case DORA_SOCK_TCP:
        DORA_EXCEPTION("no support epoller events type");
        break;

    default:
        DORA_EXCEPTION("no support epoller events type");
        break;
    }   
}

static void *epoller_thread(void *arg)
{
    int cnt = -1;
    int key = -1;
    epoller *e = (epoller *)arg;

    epoller_events_t *events = NULL;
    int happen_events = 0;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); // allow thread to exit 
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // settings cancel ASYNCHRONOUS

    while (e->_is_running) {
        DORA_LOG_DEBUG("epoller thread running ...");
        cnt = e->epoller_wait();
        if(cnt < 0) {
            DORA_EXCEPTION("epoller wait fail");
        }

        DORA_LOG_INFO("epoller wait return {}", cnt);

        for(int i = 0; i < cnt; i++)
        {
            happen_events = 0;      // clear, then get what happen events
            key = e->epoller_getfd(i, &happen_events);
            events = e->epoller_event_find(key);
            if(NULL != events) {
                epoller_events_handler(events, happen_events);
            }
            else {
                DORA_LOG_DEBUG("not find epoller events ...");
            }
        }
    }
    DORA_LOG_INFO("epoller thread exit ...");
    return NULL;
}

epoller::epoller()
    : _epollfd(epoll_create1(EPOLL_CLOEXEC))
    , _active_events(maxevents)
    , _is_running(true)
{
    if(_epollfd<0) {
        DORA_EXCEPTION("create epoller error");
        return;
    }
    // , _thread(std::thread(epoller_thread, this))
    int ret = pthread_create(&_thread, NULL, &epoller_thread, this);
    if(ret != 0) {
        DORA_EXCEPTION("create epoller thread error");
    }
    DORA_LOG_DEBUG("epoller fd is {}", _epollfd);
}

/* try not to delete it, it may cause the epoller thread
    to exit without completing execution */
epoller::~epoller()
{
    _is_running = false;

    epoller_del();
    
    pthread_cancel(_thread);
    pthread_detach(_thread);
}

int epoller::epoller_update(int operation, int fd, int events)
{
    int rst;
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = events;
    event.data.fd = fd;
    rst = epoll_ctl(_epollfd, operation, fd, &event);
    DORA_LOG_DEBUG("epoller update, operation : {}, fd : {}, res : {}", operation, fd, rst);
    return rst;
}

int epoller::epoller_getfd(int index, int *happen_events)
{
    int fd = -1;
    int size = (int)_active_events.size();
    if (size >= index) {
        fd = _active_events[index].data.fd;
        *happen_events = _active_events[index].events;
    }
    DORA_LOG_DEBUG("epoller get fd is {}", fd);
    return fd;
}

epoller_events_t* epoller::epoller_event_find(int key) 
{
    std::map<int, epoller_events_t*>::iterator it;

    it = _events_map.find(key);
    if(it != _events_map.end()) {
        return it->second;
    }
    return NULL;
}

void epoller::epoller_event_add_cb(epoller_events_t *e, epoller_event_type_t type, dgram_event_cb cb)
{
    switch (type) {
    case DORA_EPOLLER_READ:
        DORA_LOG_DEBUG("add epoller read event");
        e->events |= (EPOLLIN | EPOLLPRI);
        e->u.dgram_event.read_cb = cb;
        break;
    case DORA_EPOLLER_WRITE:
        DORA_LOG_DEBUG("add epoller write event");
        e->events |= EPOLLOUT;
        e->u.dgram_event.write_cb = cb;
        break;
    case DORA_EPOLLER_CLOSE:
        DORA_LOG_DEBUG("add epoller close event");
        e->events |= EPOLLHUP;
        e->u.dgram_event.close_cb = cb;
        break;
    case DORA_EPOLLER_ERROR:
        DORA_LOG_DEBUG("add epoller error event");
        e->events |= EPOLLERR;
        e->u.dgram_event.error_cb = cb;
        break;
    default:
        DORA_EXCEPTION("unkonw event type!");
        break;
    }
}

int epoller::epoller_add(dgram_socket *dgram, epoller_event_type_t type, dgram_event_cb cb)
{
    epoller_events_t *events = epoller_event_find(dgram->getfd());
    if (NULL != events) {
        epoller_event_add_cb(events, type, cb);
        epoller_update(EPOLL_CTL_MOD, dgram->getfd(), events->events);
        return DORA_SUCCESS_ERROR;
    }
    
    events = new epoller_events_t;
    DORA_ROBUSTNESS_CHECK(events, DORA_MEM_NOT_ENOUGH_ERROR);
    memset(events, 0, sizeof(epoller_events_t));
    events->type = DORA_SOCK_UDP;
    events->u.dgram_event.dgram = dgram;

    /* frist need insert map */
    _events_map.insert(make_pair(dgram->getfd(), events));

    DORA_LOG_DEBUG("insert events map fd {} ", dgram->getfd());

    /* register callback function on events and update epoll wait events */
    epoller_event_add_cb(events, type, cb);
    epoller_update(EPOLL_CTL_ADD, dgram->getfd(), events->events);

    return DORA_SUCCESS_ERROR;
}

int epoller::epoller_del(dgram_socket *dgram)
{
    epoller_events_t *events = epoller_event_find(dgram->getfd());
    if (NULL != events) {
        epoller_update(EPOLL_CTL_DEL, dgram->getfd(), events->events);
        delete events;
    }
    return DORA_SUCCESS_ERROR;
}

void epoller::epoller_del(void)
{
    epoller_events_t *events;
    std::map<int, epoller_events_t*>::iterator it;

    for(auto & t : _events_map) {
        events = t.second;
        if (DORA_SOCK_UDP == events->type) {
            epoller_del(events->u.dgram_event.dgram);
        }
        else if (DORA_SOCK_TCP == events->type) {
            // epoller_del(events->u.stream_event.stream);
        }
    }
}

int epoller::epoller_wait(int timeout)
{
    DORA_LOG_DEBUG("epller wait epoll fd is {}, active events size is {}", _epollfd, _active_events.size());
    return epoll_wait(_epollfd, &*_active_events.begin(), _active_events.size(), timeout);
}

}