/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-11-02 14:43:11
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#ifndef _DORA_DGRAM_H_
#define _DORA_DGRAM_H_

#include "dora_socket.h"

namespace doralib 
{
class dgram_socket {
    private:
    sockaddr_in _remote_addr;
    sockaddr_in _local_addr;
    sock *_s;

    int dgram_set_addr(sockaddr_in & sockaddr, const char *addr, const char *port);
    int dgram_set_addr(sockaddr_in & sockaddr, const std::string & addr, const std::string & port);

    public:
    dgram_socket();
    dgram_socket(const sockaddr_in & addr);
    dgram_socket(const char *addr, const char *port);
    dgram_socket(const std::string & addr, const std::string & port);
    ~dgram_socket();

    int dgram_connect(void);
    int dgram_connect(const sockaddr_in *addr);
    int dgram_connect(const sockaddr_in & addr);
    int dgram_connect(const char *addr, const char *port);
    int dgram_connect(const std::string & addr, const std::string & port);
    
    int dgram_bind(const char *port);
    int dgram_bind(const sockaddr_in *addr);
    int dgram_bind(const sockaddr_in & addr);
    int dgram_bind(const char *addr, const char *port);
    int dgram_bind(const std::string & addr, const std::string & port);

    ssize_t dgram_recvfrom(void *buf, size_t len, struct sockaddr *src_addr, socklen_t *addrlen, int timeout = -1, int flags = 0);
    
    ssize_t dgram_sendto(const void *buf, size_t len, int timeout = -1, int flags = 0);
    ssize_t dgram_sendto(const void *buf, size_t len, const char *addr, const char *port, int timeout = -1, int flags = 0);
    ssize_t dgram_sendto(const void *buf, size_t len, const std::string & addr, const std::string & port, int timeout = -1, int flags = 0);
    ssize_t dgram_sendto(const void *buf, size_t len, struct sockaddr *dest_addr, socklen_t addrlen, int timeout = -1, int flags = 0);
    
    void dgram_close(void);
    
    int getfd(void) { return _s->sock_get_sockfd(); }

};

}
#endif
