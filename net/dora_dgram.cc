/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-11-02 14:43:30
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_dgram.h"
#include "dora_error.h"
#include "dora_log.h"

namespace doralib {

int dgram_socket::dgram_set_addr(sockaddr_in & sockaddr, const char *addr, const char *port)
{
    int ret = DORA_SOCKET_UNKNOWN_HOST;
    struct addrinfo hints, *addr_list, *cur;

    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    
    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = _s->sock_get_type();
    hints.ai_protocol = IPPROTO_UDP;
    
    if (getaddrinfo(addr, port, &hints, &addr_list) != 0) {
        DORA_LOG_ERROR("can not get addr info ...");
        return ret;
    }

    if (getaddrinfo((const char *)addr, (const char *)port, &hints, &addr_list) != 0) {
        DORA_LOG_ERROR("can not get addr info ...");
        return ret;
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        switch (cur->ai_family) {
        case AF_UNSPEC:
            DORA_LOG_ERROR("unspecified");
            break;
        case AF_INET:
            struct sockaddr_in *addr;
            addr = (struct sockaddr_in *)cur->ai_addr;
            memcpy(&sockaddr, addr, sizeof(struct sockaddr_in));
            ret = DORA_SUCCESS;
            break;
        case AF_INET6:
            DORA_LOG_ERROR("not support!");
        }
    }

    freeaddrinfo(addr_list);

    sockaddr.sin_family = _s->sock_get_family();
    sockaddr.sin_port = htons(atoi((const char *)port));

    return ret;
}

int dgram_socket::dgram_set_addr(sockaddr_in & sockaddr, const std::string & addr, const std::string & port)
{
    int ret = DORA_SOCKET_UNKNOWN_HOST;
    struct addrinfo hints, *addr_list, *cur;

    bzero(&sockaddr,sizeof(sockaddr));
    
    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = _s->sock_get_type();
    hints.ai_protocol = IPPROTO_UDP;
    
    if (getaddrinfo((const char *)addr.c_str(), (const char *)port.c_str(), &hints, &addr_list) != 0) {
        DORA_LOG_ERROR("can not get addr info ...");
        return ret;
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        switch (cur->ai_family) {
        case AF_UNSPEC:
            DORA_LOG_ERROR("unspecified");
            break;
        case AF_INET:
            struct sockaddr_in *addr;
            addr = (struct sockaddr_in *)cur->ai_addr;
            memcpy(&sockaddr, addr, sizeof(struct sockaddr_in));
            ret = DORA_SUCCESS;
            break;
        case AF_INET6:
            DORA_LOG_ERROR("not support!");
        }
    }

    freeaddrinfo(addr_list);

    sockaddr.sin_family = _s->sock_get_family();
    sockaddr.sin_port = htons(atoi((const char *)port.c_str()));

    return ret;
}

dgram_socket::dgram_socket()
{
    bzero(&_remote_addr, sizeof(_remote_addr));
    bzero(&_local_addr, sizeof(_local_addr));
    _s = new sock(DORA_SOCK_UDP, DORA_SOCK_IPv4, 0);
    _s->sock_socket();
}

dgram_socket::dgram_socket(const sockaddr_in &addr)
{
    bzero(&_remote_addr,sizeof(_remote_addr));
    bzero(&_local_addr, sizeof(_local_addr));
    _remote_addr = addr;
    _s = new sock(DORA_SOCK_UDP, DORA_SOCK_IPv4, 0);
    _s->sock_socket();
}

dgram_socket::dgram_socket(const char *addr, const char *port)
{
    bzero(&_remote_addr,sizeof(_remote_addr));
    bzero(&_local_addr, sizeof(_local_addr));
    _s = new sock(DORA_SOCK_UDP, DORA_SOCK_IPv4, 0);
    _s->sock_socket();

    dgram_set_addr(_remote_addr, addr, port);
}

dgram_socket::dgram_socket(const std::string & addr, const std::string & port)
{
    bzero(&_remote_addr, sizeof(_remote_addr));
    bzero(&_local_addr, sizeof(_local_addr));
    _s = new sock(DORA_SOCK_UDP, DORA_SOCK_IPv4, 0);
    _s->sock_socket();

    dgram_set_addr(_remote_addr, addr, port);
}

dgram_socket::~dgram_socket()
{
    delete _s;
}

int dgram_socket::dgram_connect(void)
{
    if (true == _s->sock_is_connected())
        return DORA_SUCCESS;

    if (!_s->sock_is_connected())
        return _s->sock_connect(_remote_addr);
    
    return DORA_SUCCESS;
}

int dgram_socket::dgram_connect(const sockaddr_in *addr)
{
    if (true == _s->sock_is_connected())
        return DORA_SUCCESS;

    if (!_s->sock_is_connected())
        return _s->sock_connect(addr);
    
    return DORA_SUCCESS;
}

int dgram_socket::dgram_connect(const sockaddr_in & addr)
{
    if (true == _s->sock_is_connected())
        return DORA_SUCCESS;

    if (!_s->sock_is_connected())
        return _s->sock_connect(addr);
    
    return DORA_SUCCESS;
}

int dgram_socket::dgram_connect(const char *addr, const char *port)
{
    int ret = DORA_SUCCESS;

    if (true == _s->sock_is_connected())
        return DORA_SUCCESS;

    if (DORA_SUCCESS != (ret = dgram_set_addr(_remote_addr, addr, port)))
        return ret;

    if (!_s->sock_is_connected())
        return _s->sock_connect(addr, port);
    
    return DORA_SUCCESS;
}

int dgram_socket::dgram_connect(const std::string & addr, const std::string & port)
{
    if (true == _s->sock_is_connected())
        return DORA_SUCCESS;

    dgram_set_addr(_remote_addr, addr, port);

    if (!_s->sock_is_connected())
        return _s->sock_connect(addr, port);
    
    return DORA_SUCCESS;
}

int dgram_socket::dgram_bind(const char *port)
{
    return dgram_bind(NULL, port);
}

int dgram_socket::dgram_bind(const sockaddr_in *addr)
{
    memcpy(&_local_addr, addr, sizeof(_local_addr));
    return _s->sock_bind(_local_addr);
}

int dgram_socket::dgram_bind(const sockaddr_in & addr)
{
    _local_addr = addr;
    return _s->sock_bind(_local_addr);
}

int dgram_socket::dgram_bind(const char *addr, const char *port)
{
    if (NULL == addr)
        addr = DORA_ADDR_ANY;
    dgram_set_addr(_local_addr, addr, port);
    return _s->sock_bind(_local_addr);
}

int dgram_socket::dgram_bind(const std::string & addr, const std::string & port)
{
    dgram_set_addr(_local_addr, addr, port);
    return _s->sock_bind(_local_addr);
}

ssize_t dgram_socket::dgram_recvfrom(void *buf, size_t len, struct sockaddr *src_addr, socklen_t *addrlen, int timeout, int flags)
{
    return _s->sock_read(buf, len, src_addr, addrlen, timeout, flags);
}

ssize_t dgram_socket::dgram_sendto(const void *buf, size_t len, int timeout, int flags)
{
    return _s->sock_write(buf, len, (struct sockaddr *)&_remote_addr, sizeof(sockaddr_in), timeout, flags);
}

ssize_t dgram_socket::dgram_sendto(const void *buf, size_t len, struct sockaddr *dest_addr, socklen_t addrlen, int timeout, int flags)
{
    return _s->sock_write(buf, len, dest_addr, addrlen, timeout, flags);
}

ssize_t dgram_socket::dgram_sendto(const void *buf, size_t len, const char *dest_addr, const char *port, int timeout, int flags)
{
    sockaddr_in sockaddr;
    dgram_set_addr(sockaddr, dest_addr, port);

    return _s->sock_write(buf, len, (struct sockaddr *)&sockaddr, sizeof(sockaddr_in), timeout, flags);
}

ssize_t dgram_socket::dgram_sendto(const void *buf, size_t len, const std::string & dest_addr, const std::string & port, int timeout, int flags)
{
    sockaddr_in sockaddr;
    dgram_set_addr(sockaddr, dest_addr, port);

    return _s->sock_write(buf, len, (struct sockaddr *)&sockaddr, sizeof(sockaddr_in), timeout, flags);
}

void dgram_socket::dgram_close(void)
{
    _s->sock_close();
}

} // namespace doralib
