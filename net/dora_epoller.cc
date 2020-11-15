/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-29 19:05:37
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-15 11:54:06
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_epoller.h"
#include "dora_error.h"
#include <functional>

namespace doralib {

using doralib::sock;

const int maxevents = 16;

static void epoller_events_handler(epoller_events_t *events, int happen_events)
{
    DORA_LOG_DEBUG("happen {} events", happen_events);

    if ((happen_events & EPOLLHUP) && !(happen_events & EPOLLIN)) {
        if (NULL != events->close_cb) {
            events->close_cb(events->s);
        }
    }
    if (happen_events & EPOLLERR) {
        if (NULL != events->error_cb) {
            events->error_cb(events->s);
        }
    }
    if (happen_events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (NULL != events->read_cb) {
            events->read_cb(events->s);
        }
    }
    if (happen_events & EPOLLOUT) {
        if (NULL != events->write_cb) {
            events->write_cb(events->s);
        }
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

        DORA_LOG_DEBUG("epoller wait return {}", cnt);

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

void epoller::epoller_event_add_cb(epoller_events_t *e, epoller_event_type_t type, sock_event_cb cb)
{
    switch (type) {
    case DORA_EPOLLER_READ:
        DORA_LOG_DEBUG("add epoller read event");
        e->events |= (EPOLLIN | EPOLLPRI);
        e->read_cb = cb;
        break;
    case DORA_EPOLLER_WRITE:
        DORA_LOG_DEBUG("add epoller write event");
        e->events |= EPOLLOUT;
        e->write_cb = cb;
        break;
    case DORA_EPOLLER_CLOSE:
        DORA_LOG_DEBUG("add epoller close event");
        e->events |= EPOLLHUP;
        e->close_cb = cb;
        break;
    case DORA_EPOLLER_ERROR:
        DORA_LOG_DEBUG("add epoller error event");
        e->events |= EPOLLERR;
        e->error_cb = cb;
        break;
    default:
        DORA_EXCEPTION("unkonw event type!");
        break;
    }
}

int epoller::epoller_add(sock *s, epoller_event_type_t type, sock_event_cb cb)
{
    epoller_events_t *events = epoller_event_find(s->sock_get_sockfd());
    if (NULL != events) {
        epoller_event_add_cb(events, type, cb);
        epoller_update(EPOLL_CTL_MOD, s->sock_get_sockfd(), events->events);
        return DORA_SUCCESS;
    }
    
    events = new epoller_events_t;
    DORA_ROBUSTNESS_CHECK(events, DORA_MEM_NOT_ENOUGH);
    memset(events, 0, sizeof(epoller_events_t));
    events->s = s;

    /* frist need insert map */
    _events_map.insert(make_pair(s->sock_get_sockfd(), events));

    DORA_LOG_DEBUG("insert events map fd {} ", s->sock_get_sockfd());

    /* register callback function on events and update epoll wait events */
    epoller_event_add_cb(events, type, cb);
    epoller_update(EPOLL_CTL_ADD, s->sock_get_sockfd(), events->events);

    return DORA_SUCCESS;
}

int epoller::epoller_del(sock *s)
{
    epoller_events_t *events = epoller_event_find(s->sock_get_sockfd());
    if (NULL != events) {
        epoller_update(EPOLL_CTL_DEL, s->sock_get_sockfd(), events->events);
        delete events;
    }
    return DORA_SUCCESS;
}

void epoller::epoller_del(void)
{
    epoller_events_t *events;
    for(auto it = _events_map.begin(); it != _events_map.end();) {
        events = it->second;
        epoller_del(events->s);
        _events_map.erase(it++);          /* delete _events_map by key */
    }
}

int epoller::epoller_wait(int timeout)
{
    DORA_LOG_DEBUG("epller wait epoll fd is {}, active events size is {}", _epollfd, _active_events.size());
    return epoll_wait(_epollfd, &*_active_events.begin(), _active_events.size(), timeout);
}

}