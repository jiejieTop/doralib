/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-28 19:24:32
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-15 11:54:43
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#ifndef _DORA_EPOLL_H_
#define _DORA_EPOLL_H_

#include <cstring>
#include <memory>
#include <vector>

#include <unistd.h>
#include <sys/epoll.h>

#include "dora_exception.h"
#include "dora_socket.h"
#include "dora_singleton.h"

using std::vector;
using doralib::sock;

namespace doralib {

typedef void (*sock_event_cb)(sock *s);

typedef enum epoller_event_type {
    DORA_EPOLLER_READ = (EPOLLIN | EPOLLPRI | EPOLLRDHUP),
    DORA_EPOLLER_WRITE = EPOLLOUT,
    DORA_EPOLLER_CLOSE = EPOLLHUP,
    DORA_EPOLLER_ERROR = EPOLLERR
} epoller_event_type_t;



typedef struct epoller_events {
    int             events;
    sock            *s;
    sock_event_cb   read_cb;
    sock_event_cb   write_cb;
    sock_event_cb   close_cb;
    sock_event_cb   error_cb;

} epoller_events_t;
    
class epoller {
   private:
    // The file descriptor used by the epoller API
    int _epollfd;
    // std::thread _thread;
    pthread_t _thread;
    // Array of structures, filled on the return of `epoll_wait`.
    std::vector<struct epoll_event> _active_events;
    std::map<int, epoller_events_t*> _events_map;

    int epoller_update(int operation,int fd, int events);
    void epoller_event_add_cb(epoller_events_t *e, epoller_event_type_t type, sock_event_cb cb);
    
    public:
    bool _is_running;

    epoller();
    epoller(const epoller &) = delete;
    ~epoller();

    epoller_events_t* epoller_event_find(int key);
    int epoller_getfd(int index, int *happen_events);
    int epoller_add(sock *s, epoller_event_type_t type, sock_event_cb cb);
    int epoller_del(sock *s);
    void epoller_del(void);  // delete all socket from _event_map
    int epoller_wait(int timeout = -1);
    void epoller_start(void);

};


}

#endif